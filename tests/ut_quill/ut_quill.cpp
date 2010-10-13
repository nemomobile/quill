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

#include <QtTest/QtTest>
#include <QImage>
#include <QDebug>
#include <QuillImageFilter>
#include <QuillImageFilterFactory>
#include <QSignalSpy>

#include "unittests.h"
#include "ut_quill.h"
#include "quillfile.h"
#include "quillundocommand.h"
#include "quillundostack.h"
ut_quill::ut_quill()
{
}

Q_DECLARE_METATYPE(QuillImage);

void ut_quill::initTestCase()
{
}

void ut_quill::cleanupTestCase()
{
}

void ut_quill::init()
{
    Quill::initTestingMode();
    Quill::setPreviewSize(0, QSize(4, 1));
}

void ut_quill::cleanup()
{
    Quill::cleanup();
}

void ut_quill::testQuill()
{
    QCOMPARE(Quill::previewLevelCount(), 1);
    QCOMPARE(Quill::previewSize(0), QSize(4, 1));
}

void ut_quill::testQuillFile()
{
    QTemporaryFile testFile;
    testFile.open();
    Unittests::generatePaletteImage().save(testFile.fileName(), "png");

    Quill::setEditHistoryCacheSize(0, 5);
    Quill::setEditHistoryCacheSize(1, 5);

    QuillFile *file = new QuillFile(testFile.fileName(), "image/png");

    QVERIFY(file);

    QCOMPARE(file->fileName(),
             testFile.fileName());

    QCOMPARE(file->displayLevel(), -1);
    file->setDisplayLevel(1);

    Quill::releaseAndWait(); // preview

    QImage initialPreview = file->image();

    QCOMPARE(initialPreview.size(), QSize(4, 1));

    QCOMPARE(file->fullImageSize(), QSize(8, 2));

    QVERIFY(!file->canUndo());
    QVERIFY(!file->canRedo());
    QCOMPARE(file->allImageLevels().count(), 1);
    QVERIFY(file->image(1).isNull());

    // let the quill construct a full-size image for us

    Quill::releaseAndWait();

    QImage initialFull = file->image();

    QCOMPARE(initialFull.size(), QSize(8, 2));
    QCOMPARE(initialFull, Unittests::generatePaletteImage());

    // run a brightness filter

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QVERIFY(filter);
    filter->setOption(QuillImageFilter::Brightness, QVariant(16));

    file->runFilter(filter);

    Quill::releaseAndWait(); // preview

    QImage laterPreview = file->image();

    QCOMPARE(laterPreview.size(), QSize(4, 1));
    QCOMPARE(laterPreview, (QImage)filter->apply(initialPreview));

    QCOMPARE(laterPreview, (QImage)file->image(0));
    QVERIFY(Unittests::compareImage(file->image(1), QImage()));

    // let the quill construct a full-size image for us

    Quill::releaseAndWait();

    QImage laterFull = file->image();
    QCOMPARE(laterFull, (QImage)file->image(1));

    QCOMPARE(laterFull.size(), QSize(8, 2));
    QCOMPARE(laterFull, (QImage)filter->apply(initialFull));

    // undo

    QVERIFY(file->canUndo());
    QVERIFY(!file->canRedo());

    file->undo();

    QCOMPARE(initialPreview, (QImage)file->image(0));
    QCOMPARE(initialFull, (QImage)file->image());

    // redo

    QVERIFY(file->canRedo());
    QVERIFY(!file->canUndo());

    file->redo();

    QCOMPARE(laterPreview, (QImage)file->image(0));
    QCOMPARE(laterFull, (QImage)file->image());

    QList<QuillImage> targetList;
    targetList.append(file->image(0));
    targetList.append(file->image());

    QCOMPARE(targetList, file->allImageLevels());

    delete file;
}

// Test cache disabling, 2 preview levels + 1 full level

void ut_quill::testDisableCache()
{
    QTemporaryFile testFile;
    testFile.open();
    Unittests::generatePaletteImage().save(testFile.fileName(), "png");

    Quill::setPreviewLevelCount(2);
    QCOMPARE(Quill::previewLevelCount(), 2);

    Quill::setPreviewSize(0, QSize(2, 1));
    Quill::setPreviewSize(1, QSize(4, 1));

    QuillFile *file = new QuillFile(testFile.fileName());

    file->setDisplayLevel(2);

    // first level preview

    Quill::releaseAndWait();

    QuillImage initialPreview = file->image();

    QCOMPARE(initialPreview.size(), QSize(2, 1));

    // mid level preview

    Quill::releaseAndWait();

    QuillImage initialMid = file->image();

    QCOMPARE(initialMid.size(), QSize(4, 1));

    // full-size image

    Quill::releaseAndWait();

    QuillImage initialFull = file->image();

    QCOMPARE(initialFull.size(), QSize(8, 2));
    QCOMPARE(initialFull, QuillImage(Unittests::generatePaletteImage()));

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QVERIFY(filter);
    filter->setOption(QuillImageFilter::Brightness, QVariant(16));

    file->runFilter(filter);

    // preview level

    Quill::releaseAndWait();

    QuillImage laterPreview = file->image();
    QVERIFY(Unittests::compareImage(laterPreview,
                                    filter->apply(initialPreview)));
    QVERIFY(Unittests::compareImage(file->image(0), laterPreview));
    QVERIFY(file->image(1).isNull());
    QVERIFY(file->image(2).isNull());

    // mid level

    Quill::releaseAndWait();

    QuillImage laterMid = file->image();
    QVERIFY(Unittests::compareImage(laterMid,
                                    filter->apply(initialMid)));
    QVERIFY(Unittests::compareImage(file->image(0), laterPreview));
    QVERIFY(Unittests::compareImage(file->image(1), laterMid));
    QVERIFY(file->image(2).isNull());

    // high level

    Quill::releaseAndWait();

    QuillImage laterFull = file->image();
    QVERIFY(Unittests::compareImage(laterFull,
                                    filter->apply(initialFull)));
    QVERIFY(Unittests::compareImage(file->image(0), laterPreview));
    QVERIFY(Unittests::compareImage(file->image(1), laterMid));
    QVERIFY(Unittests::compareImage(file->image(2), laterFull));

    // undo - preview is regenerated

    file->undo();

    Quill::releaseAndWait();

    QVERIFY(Unittests::compareImage(file->image(0), initialPreview));
    QVERIFY(file->image(1).isNull());
    QVERIFY(file->image(2).isNull());

    // mid level regenerated

    Quill::releaseAndWait();

    QCOMPARE(initialPreview, file->image(0));
    QCOMPARE(initialMid, file->image(1));
    QCOMPARE(file->image(2), QuillImage());

    // high level regenerated

    Quill::releaseAndWait();

    QCOMPARE(initialPreview, file->image(0));
    QCOMPARE(initialMid, file->image(1));
    QCOMPARE(initialFull, file->image(2));

    delete file;
}

void ut_quill::testSignals()
{
    QTemporaryFile testFile;
    testFile.open();
    Unittests::generatePaletteImage().save(testFile.fileName(), "png");

    Quill::setPreviewSize(0, QSize(2, 1));
    Quill::setPreviewLevelCount(2);

    QuillFile *file = new QuillFile(testFile.fileName());

    QSignalSpy spy(file, SIGNAL(imageAvailable(const QuillImageList)));

    QuillImage image =
        QImage(testFile.fileName());
    QuillImage midImage = image.scaled(QSize(4, 1),
                                       Qt::IgnoreAspectRatio,
                                       Qt::SmoothTransformation);
    QuillImage smallImage = image.scaled(QSize(2, 1),
                                         Qt::IgnoreAspectRatio,
                                         Qt::SmoothTransformation);

    file->setDisplayLevel(2);
    Quill::releaseAndWait(); // preview 0

    QCOMPARE(spy.count(), 1);
    QuillImage spyImage = spy.last().first().value<QuillImageList>().first();

    QVERIFY(Unittests::compareImage(spyImage, smallImage));
    QCOMPARE(spyImage.z(), 0);

    Quill::releaseAndWait(); // preview 1

    QCOMPARE(spy.count(), 2);
    spyImage = spy.last().first().value<QuillImageList>().first();

    QVERIFY(Unittests::compareImage(spyImage, midImage));
    QCOMPARE(spyImage.z(), 1);

    Quill::releaseAndWait(); // full image

    QCOMPARE(spy.count(), 3);
    spyImage = spy.last().first().value<QuillImageList>().first();
    QVERIFY(Unittests::compareImage(spyImage, image));
    QCOMPARE(spyImage.z(), 2);

    // run brightness + contrast filters

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QuillImageFilter *filter2 =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QVERIFY(filter);
    filter->setOption(QuillImageFilter::Brightness, QVariant(20));

    QVERIFY(filter2);
    filter2->setOption(QuillImageFilter::Contrast, QVariant(25));

    file->runFilter(filter);
    file->runFilter(filter2);

    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait(); // preview 0

    QCOMPARE(spy.count(), 4);
    spyImage = spy.last().first().value<QuillImageList>().first();

    QVERIFY(Unittests::compareImage(spyImage,
                                    filter2->apply(filter->apply(smallImage))));
    QCOMPARE(spyImage.z(), 0);

    Quill::releaseAndWait();
    Quill::releaseAndWait(); // preview 1

    QCOMPARE(spy.count(), 5);
    spyImage = spy.last().first().value<QuillImageList>().first();

    QVERIFY(Unittests::compareImage(spyImage,
                                    filter2->apply(filter->apply(midImage))));
    QCOMPARE(spyImage.z(), 1);

    Quill::releaseAndWait(); // full image

    QCOMPARE(spy.count(), 6);
    spyImage = spy.last().first().value<QuillImageList>().first();
    QVERIFY(Unittests::compareImage(spyImage,
                                    filter2->apply(filter->apply(image))));
    QCOMPARE(spyImage.z(), 2);

    delete file;
}

void ut_quill::testLoadSave()
{
    QTemporaryFile testFile;
    testFile.open();

    Unittests::generatePaletteImage().save(testFile.fileName(), "png");

    Quill::setEditHistoryPath("/tmp/quill/history");

    QuillImage image(QImage(testFile.fileName()));
    QCOMPARE(image, QuillImage(Unittests::generatePaletteImage()));

    QuillFile *file = new QuillFile(testFile.fileName(), "png");
    QSignalSpy spy(file, SIGNAL(saved()));
    QSignalSpy generalSpy(Quill::instance(), SIGNAL(saved(QString)));

    file->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();
    QCOMPARE(file->image(), image);

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QVERIFY(filter);
    filter->setOption(QuillImageFilter::Brightness, QVariant(20));

    QuillImage targetImage = filter->apply(image);

    // construct a copy of the filter, since original filter will be lost

    QuillImageFilter *backupFilter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QVERIFY(backupFilter);
    backupFilter->setOption(QuillImageFilter::Brightness, QVariant(20));

    file->runFilter(filter);
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    QVERIFY(!Quill::isSaveInProgress());

    file->save();
    QVERIFY(Quill::isSaveInProgress());

    Quill::releaseAndWait();
    QVERIFY(!Quill::isSaveInProgress());

    QCOMPARE(spy.count(), 1);
    QCOMPARE(generalSpy.count(), 1);
    QCOMPARE(generalSpy.at(0).at(0).toString(), testFile.fileName());

    QImage loadedImage(testFile.fileName(), "png");
    QImage origImage(file->originalFileName());

    QVERIFY(Unittests::compareImage(origImage, image));
    QVERIFY(Unittests::compareImage(loadedImage, targetImage));

    // Destroy state here, simulates program close and reopen.
    Quill::cleanup();
    Quill::initTestingMode();

    Quill::setPreviewSize(0, QSize(4, 1));
    Quill::setEditHistoryPath("/tmp/quill/history");
    Quill::setEditHistoryCacheSize(0, 5);

    QuillFile *file2 = new QuillFile(testFile.fileName(), "png");
    file2->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();

    QVERIFY(Unittests::compareImage(file2->image(), loadedImage));

    // Check that edit history is preserved

    file2->undo();
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    QVERIFY(Unittests::compareImage(file2->image(), image));

    // Redo and check repeated loading

    file2->redo();
    Quill::releaseAndWait();

    QVERIFY(Unittests::compareImage(file2->image(), loadedImage));

    delete backupFilter;
    delete file;
    delete file2;
}

void ut_quill::testMultiSave()
{
    QTemporaryFile testFile;
    testFile.open();

    QTemporaryFile originFile;
    originFile.open();

    Unittests::generatePaletteImage().save(testFile.fileName(), "png");

    Quill::setEditHistoryPath("/tmp/quill/history");

    QImage image(testFile.fileName());
    QVERIFY(Unittests::compareImage(image, Unittests::generatePaletteImage()));

    QuillFile *file = new QuillFile(testFile.fileName(), "png");
    QSignalSpy spy(file, SIGNAL(saved()));

    file->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();
    QCOMPARE((QImage)file->image(), image);

    QuillImageFilter *brightnessFilter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QVERIFY(brightnessFilter);
    brightnessFilter->setOption(QuillImageFilter::Brightness, QVariant(20));

    QuillImageFilter *contrastFilter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QVERIFY(contrastFilter);
    contrastFilter->setOption(QuillImageFilter::Contrast, QVariant(20));

    file->runFilter(brightnessFilter);
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    QImage imageBetween = file->image();

    file->runFilter(contrastFilter);
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    QImage imageAfter = file->image();

    file->save();
    Quill::releaseAndWait();

    QCOMPARE(spy.count(), 1);

    Quill::cleanup();
    Quill::initTestingMode();

    Quill::setPreviewSize(0, QSize(4, 1));
    Quill::setEditHistoryPath("/tmp/quill/history");

    QuillFile *file2 = new QuillFile(testFile.fileName(), "png");
    QSignalSpy spy2(file2, SIGNAL(saved()));
    file2->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();

    QVERIFY(Unittests::compareImage(file2->image(), imageAfter));

    file2->undo();
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();//load
    QVERIFY(Unittests::compareImage(file2->image(), imageBetween));

    file2->save();
    Quill::releaseAndWait();

    QCOMPARE(spy2.count(), 1);

    Quill::cleanup();
    Quill::initTestingMode();

    Quill::setPreviewSize(0, QSize(4, 1));

    Quill::setEditHistoryPath("/tmp/quill/history");

    QuillFile *file3 = new QuillFile(testFile.fileName(), "png");
    file3->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();

    QVERIFY(Unittests::compareImage(file3->image(), imageBetween));

    delete file;
    delete file2;
    delete file3;
}

// Test situations where nothing should be saved!
void ut_quill::testNoSave()
{
    QTemporaryFile testFile;
    testFile.open();

    Unittests::generatePaletteImage().save(testFile.fileName(), "png");

    Quill::setEditHistoryPath("/tmp/quill/history");

    QImage image(testFile.fileName());
    QVERIFY(Unittests::compareImage(image, Unittests::generatePaletteImage()));

    QuillFile *file = new QuillFile(testFile.fileName(), "png");
    QSignalSpy spy(file, SIGNAL(saved()));

    file->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file->save();

    QVERIFY(Unittests::compareImage(QImage(testFile.fileName(), "png"), image));

    QCOMPARE(spy.count(), 0);

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    QVERIFY(filter);
    filter->setOption(QuillImageFilter::Brightness, QVariant(20));

    QuillImage imageAfter = filter->apply(image);

    QuillFile *file2 = new QuillFile(testFile.fileName(), "png");

    file2->runFilter(filter);
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file2->save();
    Quill::releaseAndWait();

    // now, as there are changes, something should happen

    QCOMPARE(spy.count(), 1);

    Quill::cleanup();
    Quill::initTestingMode();

    Quill::setPreviewSize(0, QSize(4, 1));
    Quill::setEditHistoryPath("/tmp/quill/history");

    QuillFile *file3 = new QuillFile(testFile.fileName(), "png");

    QSignalSpy spy2(file3, SIGNAL(saved()));

    file3->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file3->save();

    // Also now, no changes should happen

    QVERIFY(Unittests::compareImage(QImage(testFile.fileName(), "png"), imageAfter));

    QCOMPARE(spy2.count(), 0);

    delete file;
    delete file2;
    delete file3;
}

// Test that the save index is properly nulled whenever changes are made
void ut_quill::testSaveIndex()
{
    QTemporaryFile testFile;
    testFile.open();

    QTemporaryFile originFile;
    originFile.open();

    Unittests::generatePaletteImage().save(testFile.fileName(), "png");

    Quill::setEditHistoryPath("/tmp/quill/history");

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    filter->setOption(QuillImageFilter::Brightness, QVariant(20));

    QuillImageFilter *filter2 =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    filter->setOption(QuillImageFilter::Contrast, QVariant(20));

    QuillImageFilter *filter3 =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    filter->setOption(QuillImageFilter::Contrast, QVariant(30));

    QImage image(testFile.fileName());
    QVERIFY(Unittests::compareImage(image, Unittests::generatePaletteImage()));

    QuillFile *file = new QuillFile(testFile.fileName(), "png");
    QSignalSpy spy(file, SIGNAL(saved()));
    file->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file->runFilter(filter);
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file->save();
    Quill::releaseAndWait();

    QCOMPARE(spy.count(), 1);

    Quill::cleanup();
    Quill::initTestingMode();

    Quill::setPreviewSize(0, QSize(4, 1));
    Quill::setEditHistoryPath("/tmp/quill/history");

    QuillFile *file2 = new QuillFile(testFile.fileName(), "png");
    QSignalSpy spy2(file2, SIGNAL(saved()));
    file2->setDisplayLevel(1);

    file2->undo();
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file2->runFilter(filter2);
    file2->runFilter(filter3);

    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file2->save();
    Quill::releaseAndWait();

    // Now, even if the save index is exactly the same as before,
    // we should see changes happen

    QCOMPARE(spy2.count(), 1);

    delete file;
    delete file2;
}

// Test that background priority works for saving
void ut_quill::testBackgroundPriority()
{
    QTemporaryFile testFile;
    testFile.open();
    QTemporaryFile testFile2;
    testFile2.open();

    QImage image = Unittests::generatePaletteImage();
    image.save(testFile.fileName(), "png");
    image.save(testFile2.fileName(), "png");

    Quill::setEditHistoryPath("/tmp/quill/history");
    Quill::setPreviewSize(0, QSize(2, 1));
    Quill::setPreviewLevelCount(2);

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    filter->setOption(QuillImageFilter::Brightness, QVariant(20));

    QuillFile *file = new QuillFile(testFile.fileName(), "png");

    QSignalSpy changedSpy(file, SIGNAL(imageAvailable(const QuillImageList)));
    QSignalSpy spy(file, SIGNAL(saved()));
    file->setDisplayLevel(2);

    Quill::releaseAndWait();
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file->runFilter(filter);
    file->save();

    file->setDisplayLevel(-1);

    QuillFile *file2 = new QuillFile(testFile2.fileName(), "png");

    QSignalSpy changedSpy2(file2, SIGNAL(imageAvailable(const QuillImageList)));
    QSignalSpy spy2(file2, SIGNAL(saved()));
    file2->setDisplayLevel(2);

    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(changedSpy2.count(), 0);

    // Filter run, preview 0 for closed file (already started)
    Quill::releaseAndWait();

    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(changedSpy2.count(), 0);

    // Open file, preview 0 for new file
    Quill::releaseAndWait();

    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(changedSpy2.count(), 1);
    QCOMPARE(spy.count(), 0);
    QCOMPARE(spy2.count(), 0);

    // Open file, preview 1 for new file
    Quill::releaseAndWait();

    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(changedSpy2.count(), 2);
    QCOMPARE(spy.count(), 0);
    QCOMPARE(spy2.count(), 0);

    // Filter run, full image for closed file
    Quill::releaseAndWait();

    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(changedSpy2.count(), 2);
    QCOMPARE(spy.count(), 0);
    QCOMPARE(spy2.count(), 0);

    // Save file, closed file
    Quill::releaseAndWait();

    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(changedSpy2.count(), 2);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy2.count(), 0);

    QFile realFile(file->originalFileName());
    QVERIFY(realFile.size() > 0);

    // Open file, full image for new file

    Quill::releaseAndWait();

    QCOMPARE(changedSpy.count(), 3);
    QCOMPARE(changedSpy2.count(), 3);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy2.count(), 0);

    delete file;
    delete file2;
}

// Ensure that the small picture in saved edit history case is coming
// from the correct version (up-to-date, instead of the original, if
// available).

void ut_quill::testLoadSaveSmallPicture()
{
    QTemporaryFile testFile;
    testFile.open();

    QImage image = Unittests::generatePaletteImage();
    image.save(testFile.fileName(), "png");

    Quill::setPreviewSize(0, QSize(8, 2));
    Quill::setEditHistoryPath("/tmp/quill/history");

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    filter->setOption(QuillImageFilter::Brightness, QVariant(20));

    QuillImageFilter *filter2 =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    filter->setOption(QuillImageFilter::Contrast, QVariant(20));

    QuillFile *file = new QuillFile(testFile.fileName(), "png");

    QSignalSpy spy(file, SIGNAL(saved()));
    file->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file->runFilter(filter);
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file->runFilter(filter2);
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    QImage imageAfter = file->image();

    file->save();
    Quill::releaseAndWait();

    QCOMPARE(spy.count(), 1);

    // Now, thrash the original image (full black)

    QImage blackImage(QSize(8, 2), QImage::Format_RGB32);
    blackImage.fill(qRgb(0, 0, 0));
    blackImage.save(file->originalFileName(), "png");

    QImage filteredBlackImage(QSize(8, 2), QImage::Format_RGB32);
    filteredBlackImage.fill(qRgb(36, 36, 36));

    Quill::cleanup();
    Quill::initTestingMode();

    Quill::setPreviewSize(0, QSize(8, 2));
    Quill::setEditHistoryPath("/tmp/quill/history");

    QuillFile *file2 =
        new QuillFile(testFile.fileName(), "png");
    file2->setDisplayLevel(1);

    // Verify the small picture (and the big picture) that they still
    // date back to the current version and not the original one.

    Quill::releaseAndWait();
    QVERIFY(Unittests::compareImage(file2->image(), imageAfter));

    Quill::releaseAndWait();
    QVERIFY(Unittests::compareImage(file2->image(), imageAfter));

    // Now, go back one step and see that everything dates back to the
    // original.
    file2->undo();
    Quill::releaseAndWait();//load
    Quill::releaseAndWait();
    Quill::releaseAndWait();
    QVERIFY(Unittests::compareImage(file2->image(), filteredBlackImage));

    Quill::releaseAndWait();
    Quill::releaseAndWait();
    QVERIFY(Unittests::compareImage(file2->image(), filteredBlackImage));

    delete file;
    delete file2;
}

// The stack should load the correct picture after saving.

void ut_quill::testUseAfterSave()
{
    QTemporaryFile testFile;
    testFile.open();

    QuillImage image = Unittests::generatePaletteImage();
    image.save(testFile.fileName(), "png");

    Quill::setPreviewLevelCount(2);
    Quill::setEditHistoryCacheSize(0, 3);

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.composite.brightness.contrast");
    filter->setOption(QuillImageFilter::Brightness, QVariant(20));

    QuillImage targetImage = filter->apply(image);

    QuillFile *file = new QuillFile(testFile.fileName(), "png");
    file->setDisplayLevel(1);

    Quill::releaseAndWait();
    Quill::releaseAndWait();

    file->runFilter(filter);
    Quill::releaseAndWait();
    Quill::releaseAndWait();

    QVERIFY(Unittests::compareImage(file->image(), targetImage));

    file->save();

    file->setDisplayLevel(0);

    Quill::releaseAndWait(); // load
    Quill::releaseAndWait(); // filter
    Quill::releaseAndWait(); // save

    QVERIFY(Unittests::compareImage(QImage(testFile.fileName()),
                                    targetImage));

    QVERIFY(!Unittests::compareImage(file->image(), targetImage));

    file->setDisplayLevel(1);

    Quill::releaseAndWait(); // should now be: load
    Quill::releaseAndWait(); // should now be: load

    QVERIFY(Unittests::compareImage(file->image(), targetImage));

    delete file;
}

int main ( int argc, char *argv[] ){
    QCoreApplication app( argc, argv );
    ut_quill test;
    return QTest::qExec( &test, argc, argv );

}
