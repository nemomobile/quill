/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Alexander Bokovoy <alexander.bokovoy@nokia.com>
**
** This file is part of the Quill package.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#ifndef __QUILL_CORE_H_
#define __QUILL_CORE_H_

#include <QObject>

#include "quill.h"

class File;
class QuillUndoCommand;
class QuillUndoStack;
class ImageCache;
class ThreadManager;
class TileCache;
class CorePrivate;

class Core : public QObject
{
Q_OBJECT

friend class ut_stack;
friend class ut_core;


public:

    Core(Quill::ThreadingMode threadingMode = Quill::ThreadingNormal);
    ~Core();

    /*!
      The initializer is called before instance() is called for the first time,
      it sets up the core.
    */

    static void init();

    /*!
      This initializer is called instead of regular init() when
      Quill needs to be start up in thread testing mode. If the Core
      has already been set up, this function will fail.
    */

    static void initTestingMode();

    /*!
      This function will destroy the Core and all its related data
      structures. All instantiated QuillFile objects will have their
      valid() property set to false.
     */

    static void cleanup();

    /*!
      The instance reference for the class.
     */

    static Core *instance();

    /*!
      Does Core have the file?
    */

    bool fileExists(const QString &fileName);

    /*!
      Opens new file for viewing and editing.
    */

    File *file(const QString &fileName,
               const QString &fileFormat);

    /*!
      Removes a file from the core.
     */

    void detach(File *file);

    /*!
      Inserts a new file.
     */

    void insertFile(File *file, const QString &key);

    /*!
      Modifies the preview level count. If new previews are created,
      both of their dimensions are twice those of the previous level
      by default. Calling this function after any files have been
      opened will fail.

      @param count The number of preview levels. Must be at least 1.
     */

    void setPreviewLevelCount(int count);

    /*!
      Gets the number of preview levels in use.
     */

    int previewLevelCount() const;

    /*!
      Gets the preview size of the given level.
    */

    QSize previewSize(int level) const;

    /*!
      Gets access to the undo cache of the given level.
    */

    ImageCache *cache(int level) const;

    /*!
      Sets the size of a preview-level image. Calling this function will fail
      if any files are open.
     */

    void setPreviewSize(int level, const QSize &size);

    /*!
      Sets the default tile size if tiling is in use.
      The default is 0, which disables tiling.
     */

    void setDefaultTileSize(const QSize &defaultTileSize);

    /*!
      Gets the default tile size if tiling is in use.
      Returns QSize() if tiling is not in use.
    */

    QSize defaultTileSize() const;

    /*!
      Sets the file limit for a given level.
     */

    void setFileLimit(int level, int limit);

    /*!
      Returns the file limit for a given level.
     */

    int fileLimit(int level) const;

    /*!
      Sets the edit history cache size for a given level.
     */

    void setEditHistoryCacheSize(int level, int limit);

    /*!
      Returns the edit history cache size for a given level.
     */

    int editHistoryCacheSize(int level);

    /*!
      Sets the tile cache size (measured in tiles, not bytes!)
      The default is 20.
     */

    void setTileCacheSize(int size);

    /*!
      Sets the maximum save buffer size, in pixels (4 bytes per pixel).
    */

    void setSaveBufferSize(int size);

    /*!
      The maximum save buffer size, in pixels (4 bytes per pixel).
    */

    int saveBufferSize() const;

    /*!
      Dumps the image editor state into a byte array which can be
      saved to the file system.
    */

    QByteArray dump() const;

    /*!
      Recovers the state of the image editor after a crash.
      All files start as closed.
    */

    void recover(QByteArray history);

    /*
      Sets the default directory where to look for edit histories.
      Default is $HOME/.config/quill/history.
     */

    void setEditHistoryDirectory(const QString &directory);

    QString editHistoryDirectory() const;

    /*!
      Sets the directory where to look for ready-made thumbnails
      for a given preview level. Default is empty, meaning that all
      preview images for the level are generated from the full image.
     */

    void setThumbnailDirectory(int level, const QString &directory);

    QString thumbnailDirectory(int level) const;

    /*!
      Sets the file extension which is used in storing and retrieving thumbnail
      files. This is also used to determine the format of the thumbnail files.
      Does not include a full stop. Default is "png".
     */

    void setThumbnailExtension(const QString &format);

    QString thumbnailExtension() const;

    /*!
      Enables or disables thumbnail creation.
     */

    void setThumbnailCreationEnabled(bool enabled);

    /*!
      Returns true if thumbnail creation is enabled. True by
      default, this can be made false by setThumbnailCreationEnabled(), or if
      thumbnail creation fails by some reason.
     */

    bool isThumbnailCreationEnabled() const;

    /*!
      To make background loading tests easier on fast machines

      Only works if Quill has been created with threadingMode =
      Quill::ThreadingTest. This will allow the background thread to
      apply one image operation, and make the foreground thread to
      wait for it. Calling this is required, or the background thread
      will freeze forever.
    */

    void releaseAndWait();

    /*!
      To make background loading tests easier on fast machines
      @param delay extra delay per operation in seconds
    */

    void setDebugDelay(int delay);

    /*
      From all stacks open and closed, finds one command by its unique id.
    */

    QuillUndoCommand *findInAllStacks(int id) const;

    /*
      Used to check if the background thread could be activated to do
      a task. If successful, will also start to do that task.
    */

    void suggestNewTask();

    /*!
      @return if the thread manager allows the deletion of a
      filter (so that it is not running on the background).
    */

    bool allowDelete(QuillImageFilter *filter) const;

    /*!
      Access to the tile cache of the core.
     */

    TileCache *tileCache() const;

    /*!
      Return the number of files which have at least a given display level.
    */

    int numFilesAtLevel(int level) const;

private:

    /*!
      Return one file.
    */

    File *priorityFile() const;

    /*!
      Return one file.
    */

    File *prioritySaveFile() const;

    /*!
      Return all existing files.
    */

    QList<File*> existingFiles() const;

private:

    static Core *g_instance;

    QList<QSize> m_previewSize;
    QList<ImageCache*> m_cache;
    QList<int> m_fileLimit;

    QString m_editHistoryDirectory;
    QList<QString> m_thumbnailDirectory;
    QString m_thumbnailExtension;
    bool m_thumbnailCreationEnabled;

    QMap<QString, File*> m_files;

    QSize m_defaultTileSize;
    int m_saveBufferSize;

    TileCache *m_tileCache;
    ThreadManager *m_threadManager;
};

#endif
