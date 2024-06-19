#/*
#Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
#SPDX-License-Identifier: X11
#*/

#include <stdint.h>

//#include "cmdlineparser.h"
#include <iostream>
#include <cstring>
#include "FPGAHandler.h"

//// XRT includes
//#include "xrt/xrt_bo.h"
//#include <experimental/xrt_xclbin.h>
//#include "xrt/xrt_device.h"
//#include "xrt/xrt_kernel.h"
//#include <experimental/xrt_queue.h>

#include "StreamInterface.h"

int main(int argc, char** argv) {
	srand(time(NULL));

	FPGAHandler* fgpainterface = FPGAHandler::getInstance();

	uint64_t cnt = 0;
	uint64_t sendcnt = 0;

	StreamInterface* ifc = fgpainterface->getStreamInterface();

	double inputtest2[64];
	double inputtest3[64];
	double postest[4];
	postest[0] = 15;
	postest[1] = 3;
	postest[2] = 4;
	postest[3] = 5;
	double postest2[4];
	postest2[0] = 1;
	postest2[1] = 1;
	postest2[2] = 1;
	postest2[3] = 1;
	for (int j = 0; j < 64; j++)
	{
		inputtest2[j] = j * 15+1;
	}
	double christoffelResult[64];
	double christoffelResult2[64];
	//printf("result0: %f\n", christoffelResult[1]);

	int key = fgpainterface->send(inputtest2, postest, 5);
	for (int j = 0; j < 64; j++)
	{
		inputtest3[j] = j * 7+1;
	}
	int key2 = fgpainterface->send(inputtest3, postest2, 5);
	fgpainterface->receive(christoffelResult, key);
	printf("result: %f\n", christoffelResult[1]);

	fgpainterface->receive(christoffelResult2, key2);
	printf("result: %f\n", christoffelResult2[1]);


	// //ChristoffelInput inputtest = {{0}, {0}, 5};
	// ChristoffelInput* inputtest = new ChristoffelInput{{0}, {0}, 5};
	// inputtest.pos[0] = 15;
	// inputtest.pos[1] = 3;
	// inputtest.pos[2] = 4;
	// inputtest.pos[3] = 5;
	// inputtest->pos[0] = 15;
	// // inputtest->pos[1] = 3;
	// // inputtest->pos[2] = 4;
	// // inputtest->pos[3] = 5;
	// // inputtest->spin = 5;
	// for (int j = 0; j < 64; j++)
	// {
	// 	//christoffelOutput[j] = 70;
	// 	inputtest->dst[j] = j * 15+1;
	// }
	// double christoffelResult[64];

	// while ( ifc->send(&inputtest, sizeof(ChristoffelInput)) < 0 ) {
	// }
	// bool flushsuccess = ifc->flush();
	// while ( ifc->recv(christoffelResult, sizeof(double[64])) > 0 ) {
	// }
	// printf("result: %f\n", christoffelResult[1]);
	// // delete inputtest;

	//printf( "SendCnt: %lu Cnt: %lu\n", sendcnt, cnt );
	printf( "total receive: %lu\n", ifc->m_totalRecvBytes );
	printf("end line------\n");
    return 0;
}
