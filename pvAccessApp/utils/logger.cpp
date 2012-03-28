/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * pvAccessCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#include <pv/logger.h>

#include <pv/noDefaultMethods.h>
#include <pv/lock.h>
#include <pv/pvType.h>

#include <epicsExit.h>
#include <pv/logger.h>

#include <fstream>
#include <iostream>
#include <time.h>
#include <cstring>

using namespace epics::pvData;
using std::ofstream;
using std::ios;
using std::endl;

#include <errlog.h>

#include <epicsTime.h>

namespace epics {
    namespace pvAccess {
        
        #define TIMETEXTLEN 32
        
        static pvAccessLogLevel g_pvAccessLogLevel = logLevelInfo;
        
        void pvAccessLog(pvAccessLogLevel level, const char* format, ...)
        {
            // TODO lock
            if (level >= g_pvAccessLogLevel)
            {
                char timeText[TIMETEXTLEN];
                epicsTimeStamp tsNow;
    
                epicsTimeGetCurrent(&tsNow);
                epicsTimeToStrftime(timeText, TIMETEXTLEN, "%Y-%m-%dT%H:%M:%S.%03f", &tsNow);
    
                printf("%s ", timeText);

                va_list arg;
                va_start(arg, format);
                vprintf(format, arg);
                va_end(arg);

                printf("\n");
            }
        }

        void pvAccessSetLogLevel(pvAccessLogLevel level)
        {
            g_pvAccessLogLevel = level;
        }

        class FileLogger : public NoDefaultMethods {
        public:
            FileLogger(String name) {
                logFile.open(name.data(), ios::app);
            }

            ~FileLogger() {
                logFile.close();
            }

            void logMessage(const char* message) {
                time_t rawtime;
                time(&rawtime);
                char* timeStr = ctime(&rawtime);
                timeStr[strlen(timeStr)-1]='\0'; // remove newline

                logFile<<timeStr<<"\t"<<message; // the newline is added by the caller
            }
        private:
            ofstream logFile;

        };

        static FileLogger* fileLogger = NULL;

        static void errLogFileListener(void* pPrivate, const char *message) {
            fileLogger->logMessage(message);
        }

        static void exitFileLoggerHandler(void* pPrivate) {
            errlogFlush();
            delete fileLogger;
        }

        void createFileLogger(String fname) {
            static Mutex mutex;
            Lock xx(mutex);

            if(fileLogger==NULL) {
                fileLogger = new FileLogger(fname);
                errlogInit(2048);
                errlogAddListener(errLogFileListener, NULL);
                epicsAtExit(exitFileLoggerHandler, NULL);
            }
        }

    }
}
