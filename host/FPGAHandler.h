#include <stdint.h>
#include <iostream>
#include <cstring>
#include "StreamInterface.h"
#include <vector>
#include <map>
#include <mutex>

struct ChristoffelInput{
    double dst[64];
    double pos[4];
    double spin;
};

struct christoffelOutput{
    double result[64];
};

#define MAX_ARRAY_LEN 1024


class FPGAHandler   //singleton class
{
    private:
        int key;
        int threshold;

		std::vector<ChristoffelInput> queueMap;
		std::vector<christoffelOutput> resultMap;
		int queueMapCnt;
		int resultMapCnt;

		StreamInterface* ifc;
        
        std::mutex mu; //adding mutex for multi-threaded
        
        static FPGAHandler* instancePtr; //For Singleton class: static pointer which will points to the instance of this class
        
        void update(){
            if ( resultMapCnt == 0 && queueMapCnt > threshold)//if the map size reach some threshold, start sending
            {   
                ChristoffelInput* christoffelInputArray = new ChristoffelInput[queueMapCnt];
                christoffelOutput* resultArray = new christoffelOutput[queueMapCnt];

                for (int i = 0; i < queueMapCnt; i++)
                {
                    christoffelInputArray[i] = queueMap[i];
                }

                while ( ifc->send(christoffelInputArray, sizeof(ChristoffelInput) * queueMapCnt) < 0 ) {}
                bool flushsuccess = ifc->flush();
                while ( ifc->recv(resultArray, sizeof(christoffelOutput) * queueMapCnt) > 0 ) {}

		        for (int i = 0; i < queueMapCnt; i++)
                {
                    resultMap[i] = resultArray[i];
					//printf( "!!" ); fflush(stdout);
                }

				resultMapCnt = queueMapCnt;
                queueMapCnt = 0;
		        
                key = 0;
                delete[] christoffelInputArray;
                delete[] resultArray;
            }
        }
    public:
        FPGAHandler()
            :queueMap(MAX_ARRAY_LEN), resultMap(MAX_ARRAY_LEN)
        {
            key = 0;
            threshold = 100; //change threshold
            ifc = StreamInterface::getInstance();
            queueMapCnt = 0;
            resultMapCnt = 0;
        }

        FPGAHandler(const FPGAHandler& obj) = delete; //For singleton class: delete copy constructor

        int send(double dst[64], double const pos[4], double const spin){
            mu.lock();

            ChristoffelInput christoffelInput;
			memcpy(christoffelInput.dst, dst, sizeof(christoffelInput.dst));
			memcpy(christoffelInput.pos, pos, sizeof(christoffelInput.pos));
            christoffelInput.spin = spin;

            queueMap[key] = christoffelInput;
            queueMapCnt++;
            int returnKey = key; //prevent other thread from changing the value of key when unlock
            key++;

            update(); //update and see if can send

            mu.unlock();

            return  returnKey; //as key
        }
        bool receive(double dst[64], int key){
            mu.lock();   //This may block the send. But we have map manipulation here.
            if (resultMapCnt > 0 )
            {
                //dst = resultMap[key].result;
                for (int i = 0; i < 64; i++){
                    dst[i] = resultMap[key].result[i];
                }

                resultMapCnt -= 1;

                mu.unlock();
                return true;
            }
            else{
                mu.unlock();
                return false; //indicate no result
            }
        }

        static FPGAHandler* getInstance() //getInstance for Singleton class
        {
          // If there is no instance of class
          // then we can create an instance.
          if (instancePtr == NULL)
          {
            instancePtr = new FPGAHandler();


            return instancePtr; 
          }
          else
          {
            return instancePtr;
          }
        }

        uint64_t getReceiveByteCount(){
            return ifc->m_totalRecvBytes;
        }

        StreamInterface* getStreamInterface(){
            return ifc;
        }

};

FPGAHandler* FPGAHandler ::instancePtr = NULL; 