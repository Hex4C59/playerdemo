#include <QDebug>
#include <QDir>

#include "playlist.h"
#include "ui_playlist.h"

#include "globalhelper.h"



Playlist::Playlist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Playlist)
{
    ui->setupUi(this);
	
}

Playlist::~Playlist()
{
    QStringList strListPlayList;
    for (int i = 0; i < ui->List->count(); i++)
    {
        strListPlayList.append(ui->List->item(i)->toolTip());
    }
    GlobalHelper::SavePlaylist(strListPlayList);

    delete ui;
}

bool Playlist::Init()
{
    if (ui->List->Init() == false)
    {
        return false;
    }

    if (InitUi() == false)
    {
        return false;
    }

    if (ConnectSignalSlots() == false)
    {
        return false;
    }

    setAcceptDrops(true);

	return true;
}

bool Playlist::InitUi()
{
    setStyleSheet(GlobalHelper::GetQssStr("://res/qss/playlist.css"));
    //ui->List->hide();
    //this->setFixedWidth(ui->HideOrShowBtn->width());
    //GlobalHelper::SetIcon(ui->HideOrShowBtn, 12, QChar(0xf104));

    ui->List->clear();

    QStringList strListPlaylist;
    GlobalHelper::GetPlaylist(strListPlaylist);

    for (QString strVideoFile : strListPlaylist)
    {
        QFileInfo fileInfo(strVideoFile);
        if (fileInfo.exists())
        {
            QListWidgetItem *pItem = new QListWidgetItem(ui->List);
            pItem->setData(Qt::UserRole, QVariant(fileInfo.filePath()));  // 用户数据
            pItem->setText(QString("%1").arg(fileInfo.fileName()));  // 显示文本
            pItem->setToolTip(fileInfo.filePath());
            ui->List->addItem(pItem);
        }
    }
    if (strListPlaylist.length() > 0)
    {
        ui->List->setCurrentRow(0);
    }

    //ui->List->addItems(strListPlaylist);


    return true;
}

bool Playlist::ConnectSignalSlots()
{
	QList<bool> listRet;
	bool bRet;

    bRet = connect(ui->List, &MediaList::SigAddFile, this, &Playlist::OnAddFile);
    listRet.append(bRet);

	for (bool bReturn : listRet)
	{
		if (bReturn == false)
		{
			return false;
		}
	}

	return true;
}

void Playlist::on_List_itemDoubleClicked(QListWidgetItem *item)
{
	emit SigPlay(item->data(Qt::UserRole).toString());
    m_nCurrentPlayListIndex = ui->List->row(item);
    ui->List->setCurrentRow(m_nCurrentPlayListIndex);
}

bool Playlist::GetPlaylistStatus()
{
    if (this->isHidden())
    {
        return false;
    }

    return true;
}

bool Playlist::IsSupportedMedia(const QString &strFileName)
{
    if (strFileName.isEmpty())
    {
        return false;
    }

    //视频：容器格式，通常含视频+音频流
    static const char *const videoExts[] = {
        ".mkv", ".rmvb", ".mp4", ".avi", ".flv", ".wmv", ".3gp",
        ".m4v", ".mov", ".ts", ".webm",
    };
    //音频：纯音频文件，播放时没有视频流（video_refresh 已做判空处理）
    static const char *const audioExts[] = {
        ".mp3", ".flac", ".wav", ".aac", ".m4a", ".ogg", ".oga",
        ".opus", ".wma", ".ape", ".ac3", ".dts", ".alac",
    };

    for (const char *ext : videoExts)
    {
        if (strFileName.endsWith(QLatin1String(ext), Qt::CaseInsensitive))
        {
            return true;
        }
    }

    for (const char *ext : audioExts)
    {
        if (strFileName.endsWith(QLatin1String(ext), Qt::CaseInsensitive))
        {
            return true;
        }
    }

    return false;
}

QString Playlist::MediaFileFilter()
{
    //与 IsSupportedMedia() 中的扩展名列表保持一致
    return QStringLiteral(
        "媒体文件(*.mkv *.mp4 *.avi *.flv *.wmv *.rmvb *.3gp *.m4v *.mov *.ts *.webm "
        "*.mp3 *.flac *.wav *.aac *.m4a *.ogg *.oga *.opus *.wma *.ape *.ac3 *.dts *.alac);;"
        "视频文件(*.mkv *.mp4 *.avi *.flv *.wmv *.rmvb *.3gp *.m4v *.mov *.ts *.webm);;"
        "音频文件(*.mp3 *.flac *.wav *.aac *.m4a *.ogg *.oga *.opus *.wma *.ape *.ac3 *.dts *.alac);;"
        "所有文件(*.*)");
}

void Playlist::OnAddFile(QString strFileName)
{
    if (!IsSupportedMedia(strFileName))
    {
        return;
    }


    QFileInfo fileInfo(strFileName);
	QList<QListWidgetItem *> listItem = ui->List->findItems(fileInfo.fileName(), Qt::MatchExactly);
    QListWidgetItem *pItem = nullptr;
	if (listItem.isEmpty())
	{
        pItem = new QListWidgetItem(ui->List);
        pItem->setData(Qt::UserRole, QVariant(fileInfo.filePath()));  // 用户数据
        pItem->setText(fileInfo.fileName());  // 显示文本
        pItem->setToolTip(fileInfo.filePath());
        ui->List->addItem(pItem);
	}
    else
    {
        pItem = listItem.at(0);
    }
}

void Playlist::OnAddFileAndPlay(QString strFileName)
{
    if (!IsSupportedMedia(strFileName))
    {
        return;
    }
    QFileInfo fileInfo(strFileName);
    QList<QListWidgetItem *> listItem = ui->List->findItems(fileInfo.fileName(), Qt::MatchExactly);
    QListWidgetItem *pItem = nullptr;
    if (listItem.isEmpty())
    {
        pItem = new QListWidgetItem(ui->List);
        pItem->setData(Qt::UserRole, QVariant(fileInfo.filePath()));  // 用户数据
        pItem->setText(fileInfo.fileName());  // 显示文本
        pItem->setToolTip(fileInfo.filePath());
        ui->List->addItem(pItem);
    }
    else
    {
        pItem = listItem.at(0);
    }
    on_List_itemDoubleClicked(pItem);
}

void Playlist::OnBackwardPlay()
{
    if (m_nCurrentPlayListIndex == 0)
    {
        m_nCurrentPlayListIndex = ui->List->count() - 1;
        on_List_itemDoubleClicked(ui->List->item(m_nCurrentPlayListIndex));
        ui->List->setCurrentRow(m_nCurrentPlayListIndex);
    }
    else
    {
        m_nCurrentPlayListIndex--;
        on_List_itemDoubleClicked(ui->List->item(m_nCurrentPlayListIndex));
        ui->List->setCurrentRow(m_nCurrentPlayListIndex);
    }
}

void Playlist::OnForwardPlay()
{
    if (m_nCurrentPlayListIndex == ui->List->count() - 1)
    {
        m_nCurrentPlayListIndex = 0;
        on_List_itemDoubleClicked(ui->List->item(m_nCurrentPlayListIndex));
        ui->List->setCurrentRow(m_nCurrentPlayListIndex);
    }
    else
    {
        m_nCurrentPlayListIndex++;
        on_List_itemDoubleClicked(ui->List->item(m_nCurrentPlayListIndex));
        ui->List->setCurrentRow(m_nCurrentPlayListIndex);
    }
}

void Playlist::dropEvent(QDropEvent *event)
{
    QList<QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
    {
        return;
    }

    for (QUrl url : urls)
    {
        QString strFileName = url.toLocalFile();

        OnAddFile(strFileName);
    }
}

void Playlist::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}
