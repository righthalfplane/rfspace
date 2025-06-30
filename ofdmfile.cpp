//
// ofdmflexframesync_example.c - From liquid-sdr
//
// Example demonstrating the OFDM flexible frame synchronizer.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <csignal>
#include <chrono>


#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>


#include <liquid/liquid.h>

//c++ -std=c++11 -o ofdmfile.x ofdmfile.cpp -lliquid -lSoapySDR -Wno-return-type-c-linkage -Wno-deprecated-declarations
//ofdmfile.x -M 32 -n 256 -t -d "driver=lime" junksend.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=hackrf"  junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=bladerf"  junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=rtlsdr"  junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=uhd"  junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -t -d "driver=lime" -f 230 junksend.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=hackrf" -f 230 -o 0.01 junkreceive.cpp

void usage()
{
    printf("ofdmflexframesync_example [options]\n");
    printf(" -h        : print usage\n");
    printf(" -s  <snr> : signal-to-noise ratio [dB], default: 20\n");
    printf(" -F <freq> : carrier frequency offset, default: 0.002\n");
    printf(" -M  <num> : number of subcarriers (must be even), default: 64\n");
    printf(" -C  <len> : cyclic prefix length, default: 16\n");
    printf(" -n  <len> : payload length [bytes], default: 120\n");
    printf(" -m  <mod> : modulation scheme (qpsk default)\n");
    liquid_print_modulation_schemes();
    printf(" -v  <crc> : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf(" -c  <fec> : coding scheme (inner): h74 default\n");
    printf(" -k  <fec> : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
    printf(" -D       : enable debugging\n");
    printf(" -d  <driver=bladerf>     : output device\n");
}

// callback function
int callback(unsigned char *  _header,
             int              _header_valid,
             unsigned char *  _payload,
             unsigned int     _payload_len,
             int              _payload_valid,
             framesyncstats_s _stats,
             void *           _userdata);
             
int loop = 0;
void sigIntHandler(const int)
{
    loop = 0;
}
int receive(unsigned int payload_len,ofdmflexframegen fg,char *fileout,
      char *outDevice,unsigned int M,unsigned int cp_len,double fcc,double foffset)
{

	FILE *out=NULL;
	
	if(fileout){
		out=fopen(fileout,"wb");
		if(!out){
			fprintf(stderr,"Could not Open File %s to write\n",fileout);
			exit(1);		
		}
	}

    // allocate memory for header, payload
    unsigned char header[8];
    unsigned char payload[payload_len];
    
     unsigned int  taper_len=4;
    
    ofdmflexframesync fs = ofdmflexframesync_create(M, cp_len, taper_len, NULL, callback, (void*)out);
   

    // initialize header/payload and assemble frame
    ofdmflexframegen_assemble(fg, header, payload, payload_len);
    ofdmflexframegen_print(fg);
    ofdmflexframesync_print(fs);
	unsigned int symbols = ofdmflexframegen_getframelen(fg);
	unsigned int symbolSize = M + cp_len;
	unsigned int frameSize = symbols * symbolSize;
	unsigned int buf_len=frameSize;
	
	float *buf=new float[buf_len*2];
    

    const double fc = fcc+foffset;  //center frequency to 230 MHz
    const double f = fcc;  //center frequency to 230 MHz
    const double sample_rate = 2000000;    //sample rate to 2 MHz
    float As = 60.0f;
    nco_crcf fShift;
    float shift=fc-f;
    fShift = nco_crcf_create(LIQUID_NCO);
    nco_crcf_set_frequency(fShift,(float) ((2.0 * M_PI) * (((double) abs(shift)) / ((double) sample_rate))));

    
     std::string argStr(outDevice);
    
    std::vector<size_t> channels;

	channels = {0};
			
    SoapySDR::Device *device2 = SoapySDR::Device::make(argStr);
    if (device2 == NULL)
    {
        fprintf(stderr,"No device!\n");
        return EXIT_FAILURE;
    }
    
    std::vector< std::string > str_list;
    
	str_list = device2->listAntennas( SOAPY_SDR_RX, 0);
	fprintf(stderr,"Rx antennas: ");
	for(int i = 0; i < str_list.size(); ++i)
		fprintf(stderr,"%s,", str_list[i].c_str());
	fprintf(stderr,"\n");

	str_list = device2->listGains( SOAPY_SDR_RX, 0);
	fprintf(stderr,"Rx Gains: ");
	for(int i = 0; i < str_list.size(); ++i)
		fprintf(stderr,"%s, ", str_list[i].c_str());
	fprintf(stderr,"\n");
    
	SoapySDR::RangeList ranges = device2->getFrequencyRange( SOAPY_SDR_RX, 0);
	fprintf(stderr,"Rx freq ranges: ");
	for(int i = 0; i < ranges.size(); ++i)
		fprintf(stderr,"[%g Hz -> %g Hz], ", ranges[i].minimum(), ranges[i].maximum());
	fprintf(stderr,"\n");

   device2->setSampleRate(SOAPY_SDR_RX, 0, sample_rate);

    fprintf(stderr, "Sample rate: %g MHz\n",sample_rate/1e6);

    //Set center frequency
    
     device2->setFrequency(SOAPY_SDR_RX, 0, fc);
    
     device2->setGain(SOAPY_SDR_RX, 0, 35.0);
     
     //device2->setAntenna(SOAPY_SDR_RX, 0, BAND1);
     
     //Streaming Setup

    SoapySDR::Stream *rxStream = device2->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
    
    size_t MTU;
    
    MTU=device2->getStreamMTU(rxStream);
    
    int ret4=device2->activateStream(rxStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    
/*    
	int hasGainMode=device2->hasGainMode(SOAPY_SDR_RX, 0);    
	fprintf(stderr,"hasGainMode: %d\n",hasGainMode);

	if(hasGainMode){
		bool automatic=true;
		device2->setGainMode(SOAPY_SDR_RX,0, automatic);
		fprintf(stderr,"getGainMode: %d\n",device2->getGainMode(SOAPY_SDR_RX, 0));
	}
*/    
    
   
	float Ratio1 = (float)(10000.0/(float)sample_rate);
	
	msresamp_crcf iqSampler1  = msresamp_crcf_create(Ratio1, As);

	printf("Ratio1 %g buf_len %d\n",Ratio1,buf_len);
	
    
    float *buf2= new float[(int)(10*sample_rate*8)];
    
    float *buf3= new float[(int)(10*sample_rate*8)];
    
    signal(SIGINT, sigIntHandler);
    
    unsigned int num1;
 
    ofdmflexframegen_assemble(fg, header, payload, payload_len);
   
    // generate frame, push through channel
    loop = 1;
	while(loop){
//        fprintf(stderr,"j = %d buf_len %d\n",j,buf_len);
	
		{
			// generate symbol
			//last_symbol = ofdmflexframegen_write(fg, (liquid_float_complex *)buf, buf_len);
			
 			unsigned int bufsize=buf_len/Ratio1;
 			
 			//unsigned int bufsize=buf_len;

 			unsigned int tosend=bufsize;

			float *out=buf2;
			
    		static FILE *out11=NULL;
/*
 			if(!out11){
 				char *name=(char *)"Receive_IQ_230000000_2000000_fc.raw";
 				out11=fopen(name,"wb");
 				if(!out11)fprintf(stderr,"open error %s\n",name);
 			}
*/
			std::vector<void *> buffs(2);
	
    		int flags(0);

			//fprintf(stderr,"start\n");
			while(1){
				long long timeNs=0;
			    int tomove;
				buffs[0] = out;
				tomove=tosend;
				if(tomove > MTU)tomove=MTU;

				//int ret = device2->writeStream(txStream,  &buffs[0], tomove, flags);
				
				int ret=0;
				
				ret = device2->readStream(rxStream, &buffs[0], tomove, flags, timeNs, 1000000L);
										
				if(out11)fwrite(out,8,ret,out11);
				
				if(ret < 0){
					fprintf(stderr, "readStream ret %d\n",ret);
					break;
				}
	
				if(ret == (int)tosend)break;
				out += 2*ret;
				tosend -= ret;
				//fprintf(stderr,"tosend %d ret %d\n",tosend,ret);
				if(tosend <= 0){
					//cout << "error: samples sent: " << tosend << "/" << num2 << endl;
					break;
				}
			}       

		if (shift >= 0) {
             nco_crcf_mix_block_up(fShift, (liquid_float_complex *)buf2, (liquid_float_complex *)buf3, bufsize);
         } else {
             nco_crcf_mix_block_down(fShift, (liquid_float_complex *)buf2, (liquid_float_complex *)buf3,bufsize);
         }
         
         for(int n=0;n<bufsize*2;++n)buf2[n]=buf3[n];

			//for(int n=0;n<buf_len;++n){
 			//	fprintf(stderr,"%d %g %g %p\n",n,buf2[2*n],buf2[2*n+1],out11);
 			//}
			//exit(1);
			     

			//fprintf(stderr,"end sample_rate %ld\n",(long)sample_rate);
			
			//fprintf(stderr,"num1 = %d buf_len = %d buf[0] %g %g\n",bufsize,buf_len,buf2[0],buf2[bufsize-1]);
			
			num1=0;

			msresamp_crcf_execute(iqSampler1, (liquid_float_complex *)buf2, bufsize, (liquid_float_complex *)buf, &num1);  // decimate

			
			//fprintf(stderr,"num2 = %d buf_len = %d buff[0] %g %g\n",num1,buf_len,buf[0],buf[buf_len-1]);


			ofdmflexframesync_execute(fs, (liquid_float_complex *)buf, buf_len);
		}
    
    }

    // export debugging file

    // destroy objects
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);
    
    device2->deactivateStream(rxStream);

    device2->closeStream(rxStream);
    
    SoapySDR::Device::unmake(device2);
    
    

    printf("done.\n");
    return 0;

	return 0;
}
 
int send(unsigned int payload_len,ofdmflexframegen fg,char *filein,
      char *outDevice,unsigned int M,unsigned int cp_len,double frequency,double foffset)
{
	FILE *in=NULL;
	
	if(filein){
		in=fopen(filein,"rb");
		if(!in){
			fprintf(stderr,"Could not Open File %s to read\n",filein);
			exit(1);		
		}
	}

    unsigned int  buf_len = 4096;
    float buf[10*buf_len*2]; // time-domain buffer

    // allocate memory for header, payload
    unsigned char header[8];
    unsigned char payload[payload_len];
    
    unsigned int  taper_len=4;
    
    ofdmflexframesync fs = ofdmflexframesync_create(M, cp_len, taper_len, NULL, callback, (void*)payload);


    // initialize header/payload and assemble frame
    for (int i=0; i<8; i++)
        header[i] = i & 0xff;
    for (int i=0; i<payload_len; i++)
        payload[i] = i & 0xff;
    ofdmflexframegen_assemble(fg, header, payload, payload_len);
    ofdmflexframegen_print(fg);
    ofdmflexframesync_print(fs);
        unsigned int symbols = ofdmflexframegen_getframelen(fg);
    	unsigned int symbolSize = M + cp_len;
    	unsigned int frameSize = symbols * symbolSize;
    	buf_len=frameSize;

    // create channel and add impairments    
    //const double frequency = 230.0e6;  //center frequency to 230 MHz
    const double sample_rate = 2000000;    //sample rate to 2 MHz
    float As = 60.0f;
    
     std::string argStr(outDevice);
    
    std::vector<size_t> channels;

	channels = {0};
	
		
    SoapySDR::Device *device2 = SoapySDR::Device::make(argStr);
    if (device2 == NULL)
    {
        fprintf(stderr,"No device!\n");
        return EXIT_FAILURE;
    }

   device2->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);

    fprintf(stderr, "Sample rate: %g MHz\n",sample_rate/1e6);

    //Set center frequency
    
     device2->setFrequency(SOAPY_SDR_TX, 0, frequency);
    
     device2->setGain(SOAPY_SDR_TX, 0, 35.0);
     
   //  device->setAntenna(SOAPY_SDR_TX, 0, BAND1);
     
     //Streaming Setup

    SoapySDR::Stream *txStream = device2->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
   
    int ret4=device2->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
   
	float Ratio1 = (float)(sample_rate/(float)10000.);
	
	msresamp_crcf iqSampler1  = msresamp_crcf_create(Ratio1, As);

	printf("Ratio1 %g buf_len %d\n",Ratio1,buf_len);
	
    
    float *buf2= new float[(int)(4*sample_rate*8)];
    
    signal(SIGINT, sigIntHandler);
    
    unsigned int num1;
    
    long int loopcount=0;
    
    auto t1 = std::chrono::high_resolution_clock::now();

    // generate frame, push through channel
    loop = 1;
	while(loop){
        //fprintf(stderr,"loop  = %d buf_len %d in %p\n",loop,buf_len,in);
        unsigned int paylen=payload_len;
    	for (int i=0; i<8; i++)
        	header[i] = i & 0xff;
    	for (int i=0; i<paylen; i++)
        	payload[i] = i & 0xff;
        if(in){
           int ret1=fread(payload,1,paylen,in);
           //fprintf(stderr,"ret1 %d\n",ret1);
           if(ret1 <= 0){
	            auto t2 = std::chrono::high_resolution_clock::now();
		        std::chrono::duration<double> difference = t2 - t1;
           		fprintf(stderr,"Rewind Input File length %ld Bytes Seconds %g loopcount %ld\n",(long)ftell(in),difference.count(),++loopcount);
           		
           		rewind(in);
           		continue;
           }
           paylen=ret1;
        }
        ofdmflexframegen_assemble(fg, header, payload, paylen);
        //fprintf(stderr,"symbols %d symbolSize %d frameSize %d\n",symbols,symbolSize,frameSize);
        
  

		//unsigned int *ifh=(unsigned int *)fg;
		int last_symbol=0;
		while (!last_symbol) {
			// generate symbol
			last_symbol = ofdmflexframegen_write(fg, (liquid_float_complex *)buf, buf_len);
			//fprintf(stderr,"%d %f %f  %f %f %ld\n",last_symbol,buf[0],buf[1],buf[2*(256-1)],buf[2*(256-1)+1],(long)ifh[16]);
//			ofdmflexframesync_execute(fs, (liquid_float_complex *)buf, buf_len);
			break;
			
		}  
		
		//ofdmflexframegen_reset(fg);

			//fprintf(stderr,"buf_len %d\n",buf_len);
			
			msresamp_crcf_execute(iqSampler1, (liquid_float_complex *)buf, buf_len, (liquid_float_complex *)buf2, &num1);
			
			//fprintf(stderr,"num1 %d\n",num1);

 			unsigned int tosend=num1;

			float *out=buf2;
			
			std::vector<void *> buffs(2);
	
    		int flags(0);
    		
    		//static FILE *out11;
 			//if(!out11)out11=fopen("Sent_IQ_230000000_2000000_fc.raw","wb");
 			//if(out11)fwrite(out,8,num1,out11);
   		
			while(1){
			    int tomove;
				buffs[0] = out;
				tomove=tosend;
				if(tomove > 25000)tomove=25000;
				
				int ret = device2->writeStream(txStream,  &buffs[0], tomove, flags);
				if(ret < 0){
					fprintf(stderr, "writeStream ret %d\n",ret);
				}
	
				if(ret == (int)tosend)break;
				out += 2*ret;
				tosend -= ret;
				if(tosend <= 0){
					//cout << "error: samples sent: " << tosend << "/" << num2 << endl;
					break;
				}
			

			//ofdmflexframesync_execute(fs, (liquid_float_complex *)buf, buf_len);
		}
		//fprintf(stderr,"num1 %u\n",num1);
    
    }


    // destroy objects
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);
    
    device2->deactivateStream(txStream);

    device2->closeStream(txStream);
    
    SoapySDR::Device::unmake(device2);
    
    fprintf(stderr,"done.\n");

	return 0;
}

int main(int argc, char*argv[])
{
    //srand(time(NULL));

    // options
    unsigned int      M               = 64;                 // number of subcarriers
    unsigned int      cp_len          = 16;                 // cyclic prefix length
    unsigned int      taper_len       = 4;                  // taper length
    unsigned int      payload_len     = 120;                // length of payload (bytes)
    modulation_scheme ms              = LIQUID_MODEM_QPSK;  // modulation scheme
    fec_scheme        fec0            = LIQUID_FEC_NONE;    // inner code
    fec_scheme        fec1            = LIQUID_FEC_HAMMING128; // outer code
    crc_scheme        check           = LIQUID_CRC_32;      // validity check
    int               debug           =  0;                 // enable debugging?
    char              *outDevice      = (char *)"driver=lime";
    char              *file           = NULL;
    int               iTransmit       = 0;
    double 			  fc			  = 230;
    double 			  foffset		  = 0;


    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"uhDs:F:M:C:n:m:v:c:k:d:tf:o:")) != EOF){
        switch (dopt) {
        case 'u':
        case 'h': usage();                    return 0;
        case 'D': debug       = 1;            break;
        case 'M': M           = atoi(optarg); break;
        case 'C': cp_len      = atoi(optarg); break;
        case 'n': payload_len = atol(optarg); break;
        case 'm': ms          = liquid_getopt_str2mod(optarg); break;
        case 'v': check       = liquid_getopt_str2crc(optarg); break;
        case 'c': fec0        = liquid_getopt_str2fec(optarg); break;
        case 'k': fec1        = liquid_getopt_str2fec(optarg); break;
        case 'd': outDevice   = optarg; break;
        case 't': iTransmit   = 1;      break;
        case 'f': fc          = atof(optarg)*1e6; break;
        case 'o': foffset     = atof(optarg)*1e6; break;
       default:
            exit(-1);
        }
        
    }
    if(optind < argc)
  	{
    	file = argv[optind];
  	} else{
    	file = NULL;
  	}

	fprintf(stderr,"file %s iTransmit %d fc %g foffset %g\n",file,iTransmit,fc,foffset);
	

    // create frame generator
    ofdmflexframegenprops_s fgprops;
    ofdmflexframegenprops_init_default(&fgprops);
    fgprops.check           = check;
    fgprops.fec0            = fec0;
    fgprops.fec1            = fec1;
    fgprops.mod_scheme      = ms;
    ofdmflexframegen fg = ofdmflexframegen_create(M, cp_len, taper_len, NULL, &fgprops);

    // create frame synchronizer

	if(iTransmit){
		send(payload_len,fg,file,outDevice,M,cp_len,fc,foffset);
	}else{
		receive(payload_len,fg,file,outDevice,M,cp_len,fc,foffset);		
	}
	
    return 0;
}

// callback function
int callback(unsigned char *  _header,
             int              _header_valid,
             unsigned char *  _payload,
             unsigned int     _payload_len,
             int              _payload_valid,
             framesyncstats_s _stats,
             void *           _userdata)
{
    fprintf(stderr,"**** callback invoked : rssi = %8.3f dB, evm = %8.3f dB, cfo = %8.5f\n", _stats.rssi, _stats.evm, _stats.cfo);

    unsigned int i;
    
    FILE *out=(FILE *)_userdata;
    
    if(out){
    	int ret1=fwrite(_payload,1,_payload_len,out);
    	if(ret1 < 0)fprintf(stderr,"Error Writing output file\n");
	}
    // print header data to standard output
    fprintf(stderr,"  header rx  :");
    for (i=0; i<8; i++)
        fprintf(stderr," %d", _header[i]);
    fprintf(stderr,"\n");

    // print payload data to standard output
   fprintf(stderr,"  payload rx :");
    for (i=0; i<_payload_len; i++) {
        fprintf(stderr," %d", _payload[i]);
        if ( ((i+1)%26)==0 && i !=_payload_len-1 )
            fprintf(stderr,"\n              ");
    }
    fprintf(stderr,"\n");

    // count errors in received payload and print to standard output
    //unsigned char * payload_tx = (unsigned char*) _userdata;
    //unsigned int num_errors = count_bit_errors_array(_payload, payload_tx, _payload_len);
   // fprintf(stderr,"  bit errors : %u / %u\n", num_errors, 8*_payload_len);

    return 0;
}

