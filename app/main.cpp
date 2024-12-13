#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

void DebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  QByteArray localMsg = msg.toLocal8Bit();

  const char* msg_type = "UNKNOWN";
  switch (type) {
  case QtDebugMsg:
    msg_type = "DEBUG";
    break;
  case QtInfoMsg:
    msg_type = "INFO";
    break;
  case QtWarningMsg:
    msg_type = "WARNING";
    break;
  case QtCriticalMsg:
    msg_type = "ERROR";
    break;
  case QtFatalMsg:
    msg_type = "FATAL";
    break;
  }

  fprintf(stderr, "[%s] %s (%s:%u)\n", msg_type, localMsg.constData(), context.function, context.line);

#ifdef Q_OS_WINDOWS
  // Windows still seems to buffer stderr and we want to see debug messages immediately, so here we make sure each line
  // is flushed
  fflush(stderr);
#endif
}

int main(int argc, char *argv[])
{
  qInstallMessageHandler(DebugHandler);

  QApplication a(argc, argv);
  a.setApplicationName(QObject::tr("SI Editor"));

  MainWindow w;

  QCommandLineParser parser;
  parser.addPositionalArgument(QCoreApplication::translate("main", "file"),
                               QCoreApplication::translate("main", "The file to open on startup."));

  parser.process(a);

  if (!parser.positionalArguments().empty()) {
    w.OpenFilename(parser.positionalArguments().first());
  }

  w.show();
  return a.exec();
}
