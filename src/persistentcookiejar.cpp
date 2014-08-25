#include "persistentcookiejar.h"

#include <QDebug>
#include <QNetworkCookie>
#include <QDir>
#include <QFile>
#include <QDesktopServices>


const char *PersistentCookieJar::line_separator = "\n";

PersistentCookieJar::PersistentCookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
{
    QString cookiejar_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);

    if (cookiejar_path.length() == 0) {
        qDebug() << "No data locations available; not storing any cookies.";
        cookiejar_location = "";
        return;
    }

    QDir cookiejar_dir(cookiejar_path);
    if (!cookiejar_dir.exists()) {
        cookiejar_dir.mkpath(".");
    }

    cookiejar_location = cookiejar_path + "/cookiejar.txt";

    load();
}

bool PersistentCookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    bool answer = QNetworkCookieJar::setCookiesFromUrl(cookieList, url);
    if (cookiejar_location.length() != 0) {
        store();
    }
    return answer;
}

void PersistentCookieJar::store() {
    qDebug() << "Writing cookies to " << cookiejar_location;

    QFile storage(cookiejar_location);
    storage.open(QIODevice::WriteOnly);

    QList<QNetworkCookie> cookies = allCookies();

    for (int i=0; i < cookies.length(); i++)
    {
        qDebug() << "Writing cookie " + i;
        storage.write(cookies[i].toRawForm());
        storage.write(line_separator);
    }

    qDebug() << "Wrote" << cookies.length() << "cookies to the cookiejar.";

    storage.flush();
    storage.close();
}

void PersistentCookieJar::load() {
    qDebug() << "Loading cookies from" << cookiejar_location;

    QFile storage(cookiejar_location);
    storage.open(QIODevice::ReadOnly);
    QByteArray bytes = storage.readAll();
    QList<QByteArray> cookies = bytes.split(*line_separator);

    int valid_cookies = 0;

    for (int i=0; i < cookies.length(); i++) {
        QList<QNetworkCookie> parsed_cookies = QNetworkCookie::parseCookies(cookies[i]);
        for (int j=0; j < parsed_cookies.length(); j++) {
            insertCookie(parsed_cookies[j]);
            valid_cookies++;
        }
    }

    qDebug() << "Read" << valid_cookies << "valid cookies from the cookiejar.";
    storage.close();
}
