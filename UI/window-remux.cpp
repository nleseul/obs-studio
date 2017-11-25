/******************************************************************************
    Copyright (C) 2014 by Ruwen Hahn <palana@stunned.de>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "window-remux.hpp"

#include "obs-app.hpp"

#include <QCloseEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QItemDelegate>
#include <QLineEdit>
#include <QMessageBox>
#include <QPainter>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolButton>

#include "qt-wrappers.hpp"

#include <memory>
#include <cmath>

using namespace std;

class RemuxPathItemDelegate : public QItemDelegate
{
public:

	enum RemuxEntryState
	{
		EmptyInputPath,
		ValidInputPath,
		InvalidInputPath
	};

	enum RemuxPathRole
	{
		EntryStateRole = Qt::ItemDataRole::UserRole
	};

	RemuxPathItemDelegate(bool isOutput, const QString &defaultPath)
		: QItemDelegate(),
		  isOutput(isOutput),
		  defaultPath(defaultPath)
	{

	}

	virtual QWidget *createEditor(QWidget *parent,
			const QStyleOptionViewItem & /* option */,
			const QModelIndex &index) const override
	{
		if (isOutput && index.data(EntryStateRole).toInt() != ValidInputPath)
		{
			return Q_NULLPTR;
		}
		else
		{
			QWidget *container = new QWidget(parent);

			QHBoxLayout *layout = new QHBoxLayout();
			layout->setMargin(0);
			layout->setSpacing(0);

			QLineEdit *text = new QLineEdit();
			text->setObjectName(QStringLiteral("text"));
			text->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding, QSizePolicy::ControlType::LineEdit));
			layout->addWidget(text);

			QToolButton *button = new QToolButton();
			button->setText("...");
			button->setSizePolicy(QSizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding, QSizePolicy::ControlType::PushButton));
			layout->addWidget(button);

			container->setLayout(layout);
			container->setFocusProxy(text);

			container->connect(button, &QToolButton::clicked, [this, container]() { const_cast<RemuxPathItemDelegate *>(this)->handleBrowse(container); });

			return container;
		}
	}

	virtual void setEditorData(QWidget *editor, const QModelIndex &index)
			const override
	{
		QLineEdit *text = editor->findChild<QLineEdit *>();
		text->setText(index.data().toString());
	}

	virtual void setModelData(QWidget *editor,
			QAbstractItemModel *model,
			const QModelIndex &index) const override
	{
		QLineEdit *text = editor->findChild<QLineEdit *>();
		model->setData(index, text->text());
	}

	virtual void paint(QPainter *painter,
			const QStyleOptionViewItem &option,
			const QModelIndex &index) const override
	{
		if (isOutput)
		{
			if (index.data(EntryStateRole) != ValidInputPath)
			{
				painter->fillRect(option.rect, option.palette.color(QPalette::ColorGroup::Disabled, QPalette::ColorRole::Background));
			}
		}
		else
		{
			if (index.data(EntryStateRole) == InvalidInputPath)
			{
				painter->fillRect(option.rect, Qt::GlobalColor::red);
			}
		}

		drawDisplay(painter, option, option.rect, index.data().toString());
		drawFocus(painter, option, option.rect);
	}

private:
	bool isOutput;
	QString defaultPath;

	void handleBrowse(QWidget *container)
	{
		static const QString RecordingPattern = "(*.flv *.mp4 *.mov *.mkv *.ts *.m3u8)";

		QLineEdit *text = container->findChild<QLineEdit *>();

		QString path = text->text();
		if (path.isEmpty())
			path = defaultPath;

		if (isOutput)
		{
			path = QFileDialog::getSaveFileName(container, QTStr("Remux.SelectTarget"),
				path, RecordingPattern);
		}
		else
		{
			path = QFileDialog::getOpenFileName(container,
				QTStr("Remux.SelectRecording"), path,
				QTStr("Remux.OBSRecording") + QString(" ") +
				RecordingPattern);
		}

		if (path.isEmpty())
			return;

		text->setText(path);

		emit commitData(container);
	}
};

OBSRemux::OBSRemux(const char *path, QWidget *parent)
	: QDialog    (parent),
	  worker     (new RemuxWorker),
	  ui         (new Ui::OBSRemux),
	  tableModel (new QStandardItemModel(1, 2)),
	  recPath    (path)
{
	ui->setupUi(this);

	ui->progressBar->setVisible(false);
	ui->buttonBox->button(QDialogButtonBox::Ok)->
			setEnabled(false);

	ui->progressBar->setMinimum(0);
	ui->progressBar->setMaximum(1000);
	ui->progressBar->setValue(0);

	tableModel->setHeaderData(0, Qt::Orientation::Horizontal, QTStr("Remux.SourceFile"));
	tableModel->setHeaderData(1, Qt::Orientation::Horizontal, QTStr("Remux.TargetFile"));
	ui->tableView->setModel(tableModel);
	ui->tableView->setItemDelegateForColumn(0, new RemuxPathItemDelegate(false, recPath));
	ui->tableView->setItemDelegateForColumn(1, new RemuxPathItemDelegate(true, recPath));
	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Stretch);
	ui->tableView->setEditTriggers(QAbstractItemView::EditTrigger::CurrentChanged);

	connect(tableModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(inputCellChanged(QStandardItem*)));

	installEventFilter(CreateShortcutFilter());

	ui->buttonBox->button(QDialogButtonBox::Ok)->
			setText(QTStr("Remux.Remux"));

	connect(ui->buttonBox->button(QDialogButtonBox::Ok),
		SIGNAL(clicked()), this, SLOT(Remux()));

	connect(ui->buttonBox->button(QDialogButtonBox::Close),
		SIGNAL(clicked()), this, SLOT(close()));

	worker->moveToThread(&remuxer);
	remuxer.start();

	//gcc-4.8 can't use QPointer<RemuxWorker> below
	RemuxWorker *worker_ = worker;
	connect(worker_, &RemuxWorker::updateProgress,
			this, &OBSRemux::updateProgress);
	connect(&remuxer, &QThread::finished, worker_, &QObject::deleteLater);
	connect(worker_, &RemuxWorker::remuxFinished,
			this, &OBSRemux::remuxFinished);
	connect(this, &OBSRemux::remux, worker_, &RemuxWorker::remux);
}

bool OBSRemux::Stop()
{
	if (!worker->job)
		return true;

	if (QMessageBox::critical(nullptr,
				QTStr("Remux.ExitUnfinishedTitle"),
				QTStr("Remux.ExitUnfinished"),
				QMessageBox::Yes | QMessageBox::No,
				QMessageBox::No) ==
			QMessageBox::Yes) {
		os_event_signal(worker->stop);
		return true;
	}

	return false;
}

OBSRemux::~OBSRemux()
{
	Stop();
	remuxer.quit();
	remuxer.wait();
}

void OBSRemux::inputCellChanged(QStandardItem *item)
{
	if (item->index().column() == 0)
	{
		QModelIndex outputIndex = tableModel->index(item->index().row(), 1);
		QFileInfo fi(tableModel->data(item->index()).toString());

		RemuxPathItemDelegate::RemuxEntryState entryState;
		if (fi.path().isEmpty())
		{
			entryState = RemuxPathItemDelegate::RemuxEntryState::EmptyInputPath;
		}
		else if (fi.exists())
		{
			entryState = RemuxPathItemDelegate::RemuxEntryState::ValidInputPath;
		}
		else
		{
			entryState = RemuxPathItemDelegate::RemuxEntryState::InvalidInputPath;
		}

		tableModel->setData(item->index(), entryState, RemuxPathItemDelegate::RemuxPathRole::EntryStateRole);
		tableModel->setData(outputIndex, entryState, RemuxPathItemDelegate::RemuxPathRole::EntryStateRole);

		if (fi.exists())
		{
			QString outputPath = fi.path() + "/" + fi.baseName() + ".mp4";

			tableModel->setData(outputIndex, outputPath);

			ui->buttonBox->button(QDialogButtonBox::Ok)->
				setEnabled(true);
		}
		else
		{
			ui->buttonBox->button(QDialogButtonBox::Ok)->
				setEnabled(false);
		}

	}
}

void OBSRemux::Remux()
{
	QString sourcePath = tableModel->index(0, 0).data().toString();
	QString targetPath = tableModel->index(0, 1).data().toString();

	if (QFileInfo::exists(targetPath))
		if (OBSMessageBox::question(this, QTStr("Remux.FileExistsTitle"),
					QTStr("Remux.FileExists")) !=
				QMessageBox::Yes)
			return;

	media_remux_job_t mr_job = nullptr;
	if (!media_remux_job_create(&mr_job, QT_TO_UTF8(sourcePath),
				QT_TO_UTF8(targetPath)))
		return;

	worker->job = job_t(mr_job, media_remux_job_destroy);
	worker->lastProgress = 0.f;

	ui->progressBar->setVisible(true);
	ui->buttonBox->button(QDialogButtonBox::Ok)->
			setEnabled(false);

	emit remux();
}

void OBSRemux::closeEvent(QCloseEvent *event)
{
	if (!Stop())
		event->ignore();
	else
		QDialog::closeEvent(event);
}

void OBSRemux::reject()
{
	if (!Stop())
		return;

	QDialog::reject();
}

void OBSRemux::updateProgress(float percent)
{
	ui->progressBar->setValue(percent * 10);
}

void OBSRemux::remuxFinished(bool success)
{
	OBSMessageBox::information(this, QTStr("Remux.FinishedTitle"),
			success ?
			QTStr("Remux.Finished") : QTStr("Remux.FinishedError"));

	worker->job.reset();
	ui->progressBar->setVisible(false);
	ui->buttonBox->button(QDialogButtonBox::Ok)->
			setEnabled(true);
}

RemuxWorker::RemuxWorker()
{
	os_event_init(&stop, OS_EVENT_TYPE_MANUAL);
}

RemuxWorker::~RemuxWorker()
{
	os_event_destroy(stop);
}

void RemuxWorker::UpdateProgress(float percent)
{
	if (abs(lastProgress - percent) < 0.1f)
		return;

	emit updateProgress(percent);
	lastProgress = percent;
}

void RemuxWorker::remux()
{
	auto callback = [](void *data, float percent)
	{
		auto rw = static_cast<RemuxWorker*>(data);
		rw->UpdateProgress(percent);
		return !!os_event_try(rw->stop);
	};

	bool success = media_remux_job_process(job.get(), callback, this);

	emit remuxFinished(os_event_try(stop) && success);
}
