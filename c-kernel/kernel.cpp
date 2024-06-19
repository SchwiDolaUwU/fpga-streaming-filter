/*
#Copyright (C) 2023, Advanced Micro Devices, Inc. All rights reserved.
#SPDX-License-Identifier: X11
#*/

#include <cstring>
#include <cmath>
#include <iostream>



void christoffel(double christoffelInput[64], double const pos[4], double spin_, double christoffelOutput[64])
{
  double dst[4][4][4];
  for (int i = 0; i < 64; i++){
	dst[i / 16][(i / 4) % 4][i % 4] = christoffelInput[i];
  }

  int a, mu, nu;
  for (a=0; a<4; ++a)
    for(mu=0; mu<4; ++mu)
      for(nu=0; nu<4; ++nu)
	dst[a][mu][nu]=0.;

  double a2_ = spin_*spin_;
  double r = pos[1];
  double sth, cth;
  sincos(pos[2], &sth, &cth);
  double
    sth2 = sth*sth, cth2 = cth*cth, sth4=sth2*sth2,
    s2th = 2.*sth*cth, c2th=cth2-sth2,
    s4th = 2.*s2th*c2th,
    s2th2= s2th*s2th, ctgth=cth/sth;
  double r2=r*r, r4=r2*r2, r6=r4*r2;
  double Sigma=r2+a2_*cth2, Sigma2=Sigma*Sigma;
  double Delta=r2-2.*r+a2_;
  double Deltam1=1./Delta,
    Sigmam1=1./Sigma,
    Sigmam2=Sigmam1*Sigmam1,
    Sigmam3=Sigmam2*Sigmam1,
    a2cthsth=a2_*cth*sth,
    rSigmam1=r*Sigmam1,
    Deltam1Sigmam2=Deltam1*Sigmam2,
    r2plusa2 = r2+a2_;

  // These formulas are taken from Semerak, MNRAS, 308, 863 (1999)
  // appendix A, and have been compared against the SageMath expressions
  // for some particular spacetime point (not in the equatorial plane);
  // they agree at machine precision

  dst[1][1][1]=(1.-r)*Deltam1+rSigmam1;
  dst[1][2][1]=dst[1][1][2]=-a2cthsth*Sigmam1;
  dst[1][2][2]=-Delta*rSigmam1;
  dst[1][3][3]=-Delta*sth2*(r+(a2_*(-2.*r2+Sigma)*sth2)/Sigma2)/Sigma;
  dst[1][3][0]=dst[1][0][3]=spin_*Delta*(-2*r2+Sigma)*sth2*Sigmam3;
  dst[1][0][0]=-Delta*(-2.*r2+Sigma)*Sigmam3;
  dst[2][1][1]=a2cthsth*Deltam1*Sigmam1;
  dst[2][2][1]=dst[2][1][2]=rSigmam1;
  dst[2][2][2]=-a2cthsth*Sigmam1;
  dst[2][3][3]=
    -sth*cth*Sigmam3 * (Delta*Sigma2 + 2.*r*r2plusa2*r2plusa2);
  dst[2][0][3]=dst[2][3][0]=spin_*r*r2plusa2*s2th*Sigmam3;
  dst[2][0][0]=-2.*a2cthsth*r*Sigmam3;
  dst[3][3][1]=dst[3][1][3]=
    Deltam1*Sigmam2 * (r*Sigma*(Sigma-2.*r) + a2_*(Sigma-2.*r2)*sth2);
  dst[3][3][2]=dst[3][2][3]=
    Sigmam2*ctgth * (-(Sigma+Delta)*a2_*sth2 + r2plusa2*r2plusa2);
  dst[3][0][1]=dst[3][1][0]=spin_*(2.*r2-Sigma)*Deltam1Sigmam2;
  dst[3][0][2]=dst[3][2][0]=-2.*spin_*r*ctgth*Sigmam2;
  dst[0][3][1]=dst[0][1][3]=
    -spin_*sth2*Deltam1Sigmam2 * (2.*r2*r2plusa2 + Sigma*(r2-a2_));
  dst[0][3][2]=dst[0][2][3]=Sigmam2*spin_*a2_*r*sth2*s2th;
  dst[0][0][1]=dst[0][1][0]=(a2_+r2)*(2.*r2-Sigma)*Deltam1Sigmam2;
  dst[0][0][2]=dst[0][2][0]=-a2_*r*s2th*Sigmam2;

  for (int i = 0; i < 64; i++){
	christoffelOutput[i] = dst[i / 16][(i / 4) % 4][i % 4];
  }

}


extern "C" {
	void kernel(
		const unsigned int *in,	// Input stream
		unsigned int *out,		// Output stream
		int in_off,				// Byte offset for input
		int out_off,			// Byte offset for output
		int size				// Size in integer. Set to output size on completion
		)
	{
#pragma HLS INTERFACE m_axi port=in depth=8192 max_read_burst_length=32  bundle=aximm1
#pragma HLS INTERFACE m_axi port=out depth=8192 max_read_burst_length=32  bundle=aximm1

		int insize = size;
		int outidx = 0;
		#pragma HLS pipeline II=1 off rewind style=stp

		size_t input_chunk_size = sizeof(double[64]) + sizeof(double[4]) + sizeof(double);
		size_t output_chunk_size = sizeof(double[64]);
		int output_to_int_ratio = output_chunk_size / sizeof(unsigned int);
		int input_to_int_ratio = input_chunk_size / sizeof(unsigned int);

		for(int i = 0; i < insize / input_chunk_size; ++i) {    //change: loop for the datatype  (done)
		//TODO
			int inidx = i;
			double christoffelInput[64];
			memcpy(christoffelInput, &in[in_off + (inidx * input_to_int_ratio)], sizeof(double[64]));
			double pos[4];
			memcpy(pos, &in[in_off + (inidx * input_to_int_ratio) + (sizeof(double[64]) / (sizeof(unsigned int)))], sizeof(double[4]));
			double spin;
			memcpy(&spin, &in[in_off + (inidx * input_to_int_ratio) + ((sizeof(double[64]) + sizeof(double[4])) / (sizeof(unsigned int)))], sizeof(double));

			//double christoffelOutput[64];
			// for (int j = 0; j < 64; j++)
			// {
			// 	//christoffelOutput[j] = 70;
			// 	//christoffelOutput[j] = christoffelInput[0] * 15+1;
			// }
			double christoffelOutput[64];
			christoffel(christoffelInput, pos, spin, christoffelOutput);

			memcpy(&out[out_off + (outidx * output_to_int_ratio) + (64 / sizeof(unsigned int))], christoffelOutput, sizeof(double[64]));

			outidx++;
		}

		out[out_off] = outidx*sizeof(double[64]) + 64; //change to the proper output size (done)
	}
}




