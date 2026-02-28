#include "widget.h"
#include <windows.h>
#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QString>
#include <QLocalSocket>
#include <io.h>
#include <fcntl.h>
#include <cstdio>

#define VERSION "V0.0"

static bool g_debugMode = false;
static QFile *g_logFile = nullptr;

void customMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    const char *prefix = "";
    switch (type) {
    case QtDebugMsg:    prefix = "[DEBUG]"; break;
    case QtInfoMsg:     prefix = "[INFO] "; break;
    case QtWarningMsg:  prefix = "[WARN] "; break;
    case QtCriticalMsg: prefix = "[ERROR]"; break;
    case QtFatalMsg:    prefix = "[FATAL]"; break;
    }
    if (g_debugMode) {
        fprintf(stderr, "%s %s\n", prefix, msg.toUtf8().constData());
        fflush(stderr);
    }
    if (g_logFile && g_logFile->isOpen()) {
        g_logFile->write(QString("%1 %2\n").arg(prefix, msg).toUtf8());
        g_logFile->flush();
    }
}

bool restartWithAdminPrivileges(const QString &appPath) {
    BOOL isAdmin = FALSE;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;
    if (AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &AdministratorsGroup)) {
        CheckTokenMembership(NULL, AdministratorsGroup, &isAdmin);
        FreeSid(AdministratorsGroup);
    }
    if (isAdmin) return true;

    SHELLEXECUTEINFO sei = { sizeof(SHELLEXECUTEINFO) };
    sei.lpVerb = L"runas";
    sei.lpFile = (LPCWSTR)appPath.utf16();
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;
    if (!ShellExecuteEx(&sei)) {
        QMessageBox::critical(nullptr, "Error", "Failed to restart as administrator.");
        return false;
    }
    return false;
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++) {
        if (qstrcmp(argv[i], "--debug") == 0) {
            g_debugMode = true;
            break;
        }
    }

    if (g_debugMode) {
        AttachConsole(ATTACH_PARENT_PROCESS);
        freopen("CONOUT$", "w", stderr);
    }

    // Install early so we catch Qt startup messages too
    qInstallMessageHandler(customMessageHandler);

    QApplication a(argc, argv);
    
    // Check for single instance
    QLocalSocket singleInstanceSocket;
    singleInstanceSocket.connectToServer("MediaControlHubPipe", QIODevice::WriteOnly);
    if (singleInstanceSocket.waitForConnected(500)) {
        QMessageBox::warning(nullptr, "HxMonitor", QString::fromUtf8("程序已经在运行中，请检查系统托盘。"));
        return 1;
    }

    // Log file path requires QApplication to exist
    QString logPath = QCoreApplication::applicationDirPath() + "/HxMonitor_debug.log";
    g_logFile = new QFile(logPath);
    g_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    qDebug() << "========================================";
    qDebug() << "  HxMonitor" << VERSION
             << (g_debugMode ? "[DEBUG MODE]" : "[NORMAL]");
    qDebug() << "  Log:" << logPath;
    qDebug() << "========================================";

    // Skip UAC elevation in debug mode
    if (!g_debugMode) {
        QString appPath = QApplication::applicationFilePath();
        if (!restartWithAdminPrivileges(appPath)) {
            return 0;
        }
    }

    Widget w;
    QString title = QString("HxMonitor ") + VERSION;
    if (g_debugMode) title += " [DEBUG]";
    w.setWindowTitle(title);
    w.show();

    int ret = a.exec();
    if (g_logFile) { g_logFile->close(); delete g_logFile; }
    return ret;
}
