#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

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
