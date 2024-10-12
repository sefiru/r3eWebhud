#pragma once

#ifndef WEB_H
#define WEB_H

extern BOOL isClientConnected;
extern BOOL isClientNew;


void startServers();
void buildHtml();
void buildResponz();
void checkIsHtmlsAreExistsAndCreateThemIfNotOrFillTheStringsWithIt();
void sendMessage(const unsigned char* chars[], int size);

#endif // WEB_H