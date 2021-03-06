#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>

#include "DRHttpSession.h"

using namespace std;


DRHttpSession::DRHttpSession(bool saveToFile) :
    toFile(saveToFile),
    reply(0),
    url(0) {
}

DRHttpSession::~DRHttpSession() {
    //do cleanup - not the same as clear()
    clear();
}

bool DRHttpSession::clear() {
    return true;
}

bool DRHttpSession::setup(QString userUrl) {
    if (!userUrl.isEmpty()) {
        url = QUrl::fromUserInput(userUrl);
        return true;
    }
    return false;
}

void DRHttpSession::execute() {
#ifdef DEBUG_READ_DATA_FROM_FILE
    size_t size;
    done(new QByteArray(readFromFile(&size)));
#else
    reply = networkAccessMgr.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()),
            this, SLOT(httpFinished()));
    // Only need this if we need to read chunks of data, but for now let's just
    // read when finished is called
    //connect(reply, SIGNAL(readyRead()),
    //        this, SLOT(httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateDataReadProgress(qint64,qint64)));
#endif
}

#ifndef DEBUG_READ_DATA_FROM_FILE
void DRHttpSession::httpFinished()
{
    // TODO: Implement and handle aborted

    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        // Propagate this to UI
        //QMessageBox::information(this, tr("HTTP"),
        //                         tr("Download failed: %1.")
        //                         .arg(reply->errorString()));
        //downloadButton->setEnabled(true);
    } else if (!redirectionTarget.isNull()) {
        // Handle redirection somehow (shouldn't be need for now)
        //QUrl newUrl = url.resolved(redirectionTarget.toUrl());
        //if (QMessageBox::question(this, tr("HTTP"),
        //                          tr("Redirect to %1 ?").arg(newUrl.toString()),
        //                          QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        //    url = newUrl;
        //    reply->deleteLater();
        //    startRequest(url);
        //    return;
        //}
    } else {

        //statusLabel->setText(tr("Downloaded %1 to %2.").arg(fileName).arg(QDir::currentPath()));
        //downloadButton->setEnabled(true);
        done(new QByteArray(reply->readAll()));
    }

    reply->deleteLater();
    reply = 0;
    finished();
}
#endif

#ifndef DEBUG_READ_DATA_FROM_FILE
void DRHttpSession::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    //if (httpRequestAborted)
    //    return;
    qDebug("DEBUG: updateDataReadProgress() bytesRead: %lld totalBytes: %lld", bytesRead, totalBytes);
    readProgress((int)bytesRead/1000, (int)totalBytes/1000);
}
#endif







#ifdef DEBUG_WRITE_DATA_TO_FILE
void DRHttpSession::dumpToFile() {
    FILE* file = 0;

    string fileStr = string("curlDump");
    int delimIdx = url.find_last_of("/");
    fileStr.append(url.substr(delimIdx+1));
    fileStr.append(".txt");

    file = fopen(fileStr.c_str(), "w");

    qDebug("DEBUG: dumpToFile() pFile=%d\n", file);

    if (file) {
        fqDebug(file, "%s\n", (const char*)this->memory);
        fclose(file);
    }
}
#endif

#ifdef DEBUG_READ_DATA_FROM_FILE
char * DRHttpSession::readFromFile(size_t *size) {
    char * buffer = 0;
    FILE * file = 0;
    string fileStr = string("/Users/dickow/git_wa/github/qt_json_parser/test/curlDump");
    string urlString = url.toString().toStdString();
    int delimIdx = urlString.find_last_of("/");
    fileStr.append(urlString.substr(delimIdx+1));
    fileStr.append(".txt");
    file = fopen(fileStr.c_str() , "r");

    qDebug("DEBUG: fileStr=%s\n", fileStr.c_str());

    if (file==0) {
        qDebug("DEBUG: fopen() error\n");
        *size = 0;
    } else {
        long lSize;
        size_t result;

        // obtain file size:
        fseek (file , 0 , SEEK_END);
        lSize = ftell (file);
        rewind (file);

        // allocate memory to contain the whole file:
        buffer = (char*) malloc (sizeof(char)*lSize);
        if (buffer == NULL) {fputs ("Memory error",stderr); exit (2);}

        // copy the file into the buffer:
        result = fread (buffer,1,lSize,file);
        if (result != (size_t)lSize) {
            qDebug("DEBUG: fread() error\n");
            *size = 0;
        } else {
            *size = (size_t)lSize;
        }
        fclose(file);
    }
    return buffer;
}
#endif
