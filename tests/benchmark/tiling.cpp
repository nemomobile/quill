/****************************************************************************
**
** Copyright (C) 2009-11 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Pekka Marjola <pekka.marjola@nokia.com>
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

#include <QCoreApplication>
#include <QEventLoop>
#include <QDir>
#include <QTime>
#include <iostream>
#include <QRect>

#include <Quill>
#include <QuillFile>
#include <QuillImageFilter>
#include <QuillImageFilterFactory>
#include "../../src/strings.h"

void tiling(QString fileName, QSize size)
{
    QEventLoop loop;

    QTime time;
    time.start();

    Quill::setPreviewSize(0, QSize(320, 200));
    Quill::setDefaultTileSize(QSize(256, 256));

    QuillFile *file = new QuillFile(fileName, Strings::jpg);
    file->setDisplayLevel(1);
    file->setViewPort(QRect(QPoint((file->fullImageSize().width() - size.width()) / 2,
                                   (file->fullImageSize().height() - size.height()) / 2),
                            size));

    QObject::connect(file, SIGNAL(imageAvailable(const QuillImageList)),
                     &loop, SLOT(quit()));

    std::cout << "Getting 256x256 tiles inside " << file->viewPort().width() << "x" << file->viewPort().height()
              << "\n";

    int tileCount = 0;

    do {
        loop.exec();

        if (tileCount > 0)
            std::cout << "Time elapsed for tile " << time.elapsed();
        else
            std::cout << "Time elapsed for setup and preview: " << time.elapsed();

        tileCount++;

        time.restart();

    } while (Quill::isCalculationInProgress());
}
