#include <QCoreApplication>
#include <QEventLoop>
#include <QDir>
#include <QTime>
#include <QDebug>
#include <QTemporaryFile>

#include <Quill>
#include <QuillFile>
#include <QuillImageFilter>
#include <QuillImageFilterFactory>

void batchrotate(QString fileName)
{
    QEventLoop loop;

    QTime time;
    time.start();

    Quill::setDefaultTileSize(QSize(256, 256));

    QFile origFile(fileName);
    origFile.open(QIODevice::ReadOnly);
    QByteArray buf = origFile.readAll();
    origFile.close();

    QTemporaryFile file;
    file.open();
    file.write(buf);
    file.flush();

    QuillFile quillFile(file.fileName(), "jpg");

    QuillImageFilter *filter =
        QuillImageFilterFactory::createImageFilter("org.maemo.rotate");
    filter->setOption(QuillImageFilter::Angle, QVariant(90));

    quillFile.runFilter(filter);

    QObject::connect(&quillFile, SIGNAL(saved()), &loop, SLOT(quit()));

    quillFile.save();

    loop.exec();

    qDebug() << "Use case batch rotate/save:" << time.elapsed() << "ms";
}