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

#pragma once

#include <QPointer>
#include <QThread>
#include <memory>
#include "ui_OBSRemux.h"

#include <media-io/media-remux.h>
#include <util/threading.h>

class QStandardItem;
class QStandardItemModel;
class RemuxWorker;

enum RemuxEntryState
{
	Empty,
	Ready,
	Pending,
	InProgress,
	Complete,
	InvalidPath,
	Error
};
Q_DECLARE_METATYPE(RemuxEntryState);

class OBSRemux : public QDialog {
	Q_OBJECT

	QThread remuxer;
	QPointer<RemuxWorker> worker;

	std::unique_ptr<Ui::OBSRemux> ui;

	const char *recPath;

	bool Stop();

	virtual void closeEvent(QCloseEvent *event) override;
	virtual void reject() override;

public:
	explicit OBSRemux(const char *recPath, QWidget *parent = nullptr);
	virtual ~OBSRemux() override;

	using job_t = std::shared_ptr<struct media_remux_job>;

protected:
	void dropEvent(QDropEvent *ev);
	void dragEnterEvent(QDragEnterEvent *ev);

private:
	QStandardItemModel *tableModel;

private slots:
	void inputCellChanged(QStandardItem *item);

public slots:
	void updateProgress(float percent);
	void updateEntryState(int key, RemuxEntryState newState);
	void remuxFinished(bool success);
	void Remux();
	void clearFinished();

signals:
	void remux();
};

class RemuxWorker : public QObject {
	Q_OBJECT

	struct JobInfo
	{
		int jobKey;

		QString sourcePath;
		QString targetPath;

		JobInfo(int key, const QString &source, const QString &target)
		: jobKey    (key),
		  sourcePath(source),
		  targetPath(target)
		{

		}
	};

	QList<JobInfo> jobQueue;

	os_event_t *wait;
	os_event_t *stop;

	float lastProgress;
	void UpdateProgress(float percent);

	explicit RemuxWorker();
	virtual ~RemuxWorker();

private slots:
	void remux();

signals:
	void updateProgress(float percent);
	void updateEntryState(int key, RemuxEntryState newState);
	void remuxFinished(bool success);

	friend class OBSRemux;
};
