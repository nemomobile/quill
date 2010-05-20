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

/*!
  \class Core

  \brief The private class for Quill, handling centralized settings
  and the file list.

Core is used to change the global settings of LibQuill. Some settings
can only be changed if no files have been opened yet.
 */

#ifndef __QUILL_CORE_H_
#define __QUILL_CORE_H_

#include <QObject>
#include <QColor>

#include "quill.h"
#include "quillerror.h"

class QuillError;
class File;
class QuillUndoCommand;
class ImageCache;
class Task;
class Scheduler;
class ThreadManager;
class TileCache;
class DBusThumbnailer;
class DisplayLevel;

class Core : public QObject
{
Q_OBJECT

friend class ut_stack;
friend class ut_core;


public:

    Core(Quill::ThreadingMode threadingMode = Quill::ThreadingNormal);

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
      Returns true if there is an instance for the file in Core.
    */

    bool fileExists(const QString &fileName);

    /*!
      Opens new file for viewing and editing.
    */

    File *file(const QString &fileName,
               const QString &fileFormat);

    /*!
      Inserts a file into the core file list.
     */

    void attach(File *file);

    /*!
      Removes a file from the core.
     */

    void detach(File *file);

    /*!
      Inserts a new file.
     */

    void insertFile(File *file, const QString &key);

    /*!
      Callback from ThreadManager.
     */

    void processFinishedTask(Task *task, QuillImage resultImage);

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
      Sets the minimum size of a preview-level image.
     */

    void setMinimumPreviewSize(int level, const QSize &size);

    /*!
      The minimum size of a preview-level image.
    */

    QSize minimumPreviewSize(int level) const;

    /*!
      Returns true if the given display level can substiture for the given
      level.
     */

    bool isSubstituteLevel(int level, int targetLevel) const;

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
      Sets the maximum allowed dimensions for an image. If either
      dimension of an image overflows its respective limit set here,
      the image will not be handled by Quill.

      This limit will not prevent image filters from resizing an image
      into bigger than the given size.

      The default is no limit, which is represented by an invalid
      size.

      See also setImagePixelsLimit().
     */

    void setImageSizeLimit(const QSize &size);

    /*!
      Returns the maximum allowed dimensions for an image. See
      setImageSizeLimit().
    */

    QSize imageSizeLimit() const;

    /*!
      Sets the maximum number of allowed pixels for an image. Images
      which have more pixels than this number will not be handled by
      Quill. This does not replace imageSizeLimit(); both limits
      are checked separately.

      The default is no limit, which is represented by the value 0.
     */

    void setImagePixelsLimit(int pixels);

    /*!
      Returns the maximum number of allowed dimensions for an image. See
      setImagePixelsLimit().
    */

    int imagePixelsLimit() const;

    /*!
      Sets the maximum number of allowed pixels for image formats which
      do not support tiling. (Currently, this means all formats except Jpeg.)
      Images which have more pixels than this number will not be handled by
      Quill. If tiling is not enabled with setDefaultTileSize(), this
      maximum size applies to all images.

      The default is no limit, which means that the generic image pixels limit
      will be used (see setImagePixelsLimit() ).
     */

    void setNonTiledImagePixelsLimit(int pixels);

    /*!
      Returns the maximum number of allowed pixels for image formats which
      do not support tiling. See setNonTilingImagePixelsLimit().
    */

    int nonTiledImagePixelsLimit() const;

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
      Enables or disables thumbnail creation using an external D-Bus
      thumbnailer (in cases of files which are not directly supported
      by Quill). The external thumbnailer must be configured to
      support all the preview levels that are specified using
      setThumbnailDirectory(). This option is true by default.
    */

    void setDBusThumbnailingEnabled(bool enabled);

    /*!
      Returns true if the external D-Bus thumbnailer has been selected to be
      used.
    */

    bool isDBusThumbnailingEnabled() const;

    /*!
      Returns true if it is possible to recover from a previous crash.
      This will only work if setCrashDumpFile() has been previously
      called, the physical file is found and actually contains something to
      recover.

      Crash recovery is only possible on a clean core. Also note that
      any successful image editing will result in overwriting the
      crash dump data.

      See also recover().
     */

    bool canRecover();

    /*!
      Recovers all unsaved edits from crash dump data stored in a file
      specified by crashDumpFile().

      The crash dump data is automatically output to a file after each
      edit. The data contains all edits which have not been
      synchronized to the file system, regardless of if
      QuillFile::save() has been called after them.

      The crash recovery feature will recover all unsaved edits and
      put them to the saving queue. Any files in the saving queue can
      be viewed or edited normally.

      This feature will not recreate the list of open files or display
      levels; an application has to keep a data structure of its own
      and to explicitly re-open any files it was viewing.
    */

    void recover();

    /*!
      Dumps all unsaved edits to the dump file.
    */

    void dump();

    /*!
      Sets the crash dump file name. Leave as the empty string to
      disable this feature (default). A directory path will be created
      for the dump file, if possible.

      See also canRecover() and recover().
     */

    void setCrashDumpPath(const QString &fileName);

    /*!
      Returns the crash dump file name. See setCrashDumpFile();
     */

    QString crashDumpPath() const;

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

    /*!
      Returns true if there is any calculation in progress.
     */

    bool isCalculationInProgress() const;

    /*!
      Returns true if there are any files which are in the progress of
      saving. Useful for example when an application wants to exit.
     */

    bool isSaveInProgress() const;

    /*!
      Sets the temporary file path
      @param fileDir the file path
    */
    void setTemporaryFileDirectory(const QString &fileDir);

    /*!
      Gets the temporary file path
     */
    QString temporaryFileDirectory() const;

    /*!
      Sets the default color for alpha channel rendering.

      As Quill does not support alpha channel editing, the alpha
      channel is immediately and permanently rendered as the
      background rendering color.
     */

    void setBackgroundRenderingColor(const QColor &color);

    /*!
      Gets the default color for alpha channel rendering.

      See setBackgroundRenderingColor().
    */

    QColor backgroundRenderingColor() const;

    /*!
      Sets the bounding box for vector graphics rendering.

      Vector graphics (mimetype "image/svg+xml") will be
      rendered to this size, preserving aspect ratio.

      Default is an invalid QSize, which means that all
      vector graphics is rendered into their intended size.
     */

    void setVectorGraphicsRenderingSize(const QSize &size);

    /*!
      Returns the bounding box for vector graphics rendering.

      Vector graphics (mimetype "image/svg+xml") will be
      rendered to this size, preserving aspect ratio.
    */

    QSize vectorGraphicsRenderingSize() const;

    /*!
      Returns the list of writable image formats. Internal use only.

      QImageWriter::supportedImageFormats() was shown to be a
      particularly heavy operation and was thus optimized with this.
    */

    QList<QByteArray> writableImageFormats();

    /*!
      Gets the target size for a given preview level for a given-size image.
     */

    QSize targetSizeForLevel(int level, const QSize &fullImageSize);

    /*!
      Gets the target area for a given preview level for a given-size image.
     */

    QRect targetAreaForLevel(int level, const QSize &targetSize,
                             const QSize &fullImageSize);

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

    /*!
      Emits a saved signal.
    */
    void emitSaved(QString fileName);

    /*!
      Emits a removed signal.
    */
    void emitRemoved(QString fileName);

    /*!
      Emits an error signal.
    */
    void emitError(QuillError error);

private:
    ~Core();

    /*!
      Activates the D-Bus thumbnailer with a task if there is any.
    */
    void activateDBusThumbnailer();

    /*!
      Converts a preview level to a D-bus thumbnailer flavor.
    */

    QString flavorFromLevel(int level);

    /*!
      Converts a D-bus thumbnailer flavor to a preview level.
    */

    int levelFromFlavor(QString flavor);

signals:
    /*!
      Edits to a file have been successfully saved.

      @param fileName the name of the file which has been saved.

      Since it may be that all QuillFile objects related to a save in
      progress have been deleted already, a successful save triggers
      both QuillFile::saved() and this signal.
     */

    void saved(QString fileName);

    /*
      A file has been removed.

      This currently applies only if the file has been explicitly removed
      with QuillFile::remove().
     */

    void removed(QString fileName);

    /*!
      Any error not specific to any individual QuillFile is reported
      by this signal.

      Since it may be that all QuillFile objects related to a save in
      progress have been deleted already, errors related to saving
      trigger both QuillFile::error() and this signal.

      @param errorCode The error code enumeration
      @param data Any other information specific to the error
    */

    void error(QuillError error);

private slots:
    void processDBusThumbnailerGenerated(const QString fileName,
                                         const QString flavor);
    void processDBusThumbnailerError(const QString fileName, uint errorCode,
                                     const QString message);

private:

    static Core *g_instance;

    QList<DisplayLevel*> m_displayLevel;

    QSize m_imageSizeLimit;
    int m_imagePixelsLimit, m_nonTiledImagePixelsLimit;
    QSize m_vectorGraphicsRenderingSize;

    QString m_editHistoryDirectory;
    QString m_thumbnailExtension;
    bool m_thumbnailCreationEnabled;
    bool m_dBusThumbnailingEnabled;
    bool m_recoveryInProgress;

    QMap<QString, File*> m_files;

    QSize m_defaultTileSize;
    int m_saveBufferSize;

    TileCache *m_tileCache;
    Scheduler *m_scheduler;
    ThreadManager *m_threadManager;

    QString m_temporaryFileDirectory;
    QString m_crashDumpPath;

    QColor m_backgroundRenderingColor;

    QList<QByteArray> m_writableImageFormats;

    DBusThumbnailer *m_dBusThumbnailer;
};

#endif
