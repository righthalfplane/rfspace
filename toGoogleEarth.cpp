#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>
#include <sys/timeb.h>

#include <stdarg.h>
#include <chrono>
#include <thread>

#include "math.h"

struct P{
    double x;
    double y;
    double z;
};

extern struct P Norm(struct P *p1);
extern struct P CrossN(struct P *v1,struct P *v2);
extern struct P Sub(struct P *p1,struct P *p2);
extern struct P Add(struct P *p1,struct P *p2);
extern struct P Mult(struct P *p1,double Factor);
extern double Len(struct P *v1);
extern double Dot(struct P *v1,struct P *v2);
extern struct P p(double x,double y,double z);
typedef double Flt ;
typedef Flt Vec[3] ;
#define VecDot(a,b)	((a)[0]*(b)[0]+(a)[1]*(b)[1]+(a)[2]*(b)[2])
#define VecLen(a)	(sqrt(VecDot(a,a)))

struct System{
	struct P x;
	struct P y;
	struct P z;
	struct P p;
	double Theta_x;
	double Theta_y;
	double Theta_z;
	double Scale_x;
	double Scale_y;
	double Scale_z;
};


struct Matrix{
	double x[4][4];
};

int Wgs84BuildLocalSystem(double latitude,double longitude,double height,struct Matrix *mf,struct Matrix *mb);
struct P Wgs84Forward(double latitude,double longitude,double height);
struct P Wgs84Reverse(double x,double y,double z);
struct P Transform(struct Matrix *m,struct P *pc);

void BuildTransformMatrix(struct Matrix *m,struct System *p1,struct System *p2);

int rotate3d(struct System *Local);

// c++ -o toGoogleEarth.x toGoogleEarth.cpp -Wall
// dump1090 | toGoogleEarth.x -outFile /Users/dir/Desktop/path.kml -dump1090
//cat shipcourse1.txt | toGoogleEarth.x -outFile /Users/dir/Desktop/path.kml -gnuais
// dump1090 | toGoogleEarth.x -outFile /home/dir/Desktop/path.kml -dump1090
//cat shipcourse1.txt | toGoogleEarth.x -outFile /home/dir/Desktop/path.kml -gnuais
//gnuais | toGoogleEarth.x -outFile /home/dir/Desktop/path.kml -gnuais


class dumpData{
	public:
	float Altitude;
	float Latitude;
	float Longitude;
	float NS;
	float EW;
	double heading;
};

class list{
	public:
	std::string name;
	std::vector <class dumpData> d;
	double timeCreate;
	double NS;
	double EW;
}l;

int writeEndDump(FILE *out,double NS,double EW,class dumpData *d,int n);

int writeEndAis(FILE *out,double heading,class dumpData *d,int n);


int nextBlockDump(char *buffin,class dumpData *d);

int nextBlockAis(char *buffin,class dumpData *d);

int nextLineDump(char *buff,int *nn);

int nextLineAis(char *buff,int *nn);

int nextLine(char *buff,int *nn);

void dump1090(char *fileName);

double rtime();

void gnuais(char *fileName);

int main(int argc,char **argv)
{
	FILE *outFile=NULL;
	char *names=NULL;

	
	if(argc < 2)return 1;

	for(int n=1;n<argc;++n){
		std::string name=argv[n];
		if(name == "-outFile"){
	    	name=argv[++n];
	    	names=argv[n];
	    	if(name == "-"){
	    		outFile=stdout;
	    		fprintf(stderr,"outFile=stdout\n");
	    	}else{
				 outFile=fopen(argv[n],"wb");
				 if(outFile == NULL){
					 fprintf(stderr,"Could Not Open %s to Write\n",argv[n]);
				 }
	         }
		}else if(name == "-dump1090"){
			if(outFile)fclose(outFile);
			outFile=NULL;
			dump1090(names);
			exit(0);
		}else if(name == "-gnuais"){
			if(outFile)fclose(outFile);
			outFile=NULL;
			gnuais(names);
			exit(0);
	    }
	}	
	
	return 0;
}
int nextBlockAis(char *buffin,class dumpData *d)
{
	char buff[2000];
	char MMSI[]={"mmsi"};
	char Lat[]={"lat"};
	char Long[]={"lon"};
	char Head[]={"head"};
	int ret;
	int nn;

	
	d->Altitude=10;
	d->Latitude=0;
	d->Longitude=0;
	d->heading=511;
	d->NS=0;
	d->EW=0;
		
	
	while(1){
		ret = nextLineAis(buff,&nn);	
		if(ret < 0){
			fprintf(stderr,"ret %d\n",ret);
			return 1;
		}
		
		fprintf(stderr,"%d %s\n",ret,buff);
		
		if(ret == 0)return 0;


		char *sp=strstr(buff,MMSI);
		if(sp){
			//fprintf(stderr,"MMSI found\n");
		}else{
			fprintf(stderr,"MMSI not found\n");
			continue;
		}
		
		char *sp2=strstr(sp,"name");
		if(sp2){
			fprintf(stderr,"name found\n");
			continue;
		}
		
		sp2=strstr(sp,Lat);
		if(sp2){
			d->Latitude=atof(sp2+4);
			//fprintf(stderr,"Lat %g\n",d->Latitude);
		}else{
			fprintf(stderr,"Lat not found\n");
			continue;
		}
		
		sp2=strstr(sp2,Long);
		if(sp2){
			d->Longitude=atof(sp2+4);
			//fprintf(stderr,"Long %g\n",d->Longitude);
		}else{
			fprintf(stderr,"Long not found\n");
			continue;
		}
			
		sp2=strstr(sp2,Head);
		if(sp2){
			d->heading=atof(sp2+8);
			//fprintf(stderr,"Head %g\n",d->heading);
			if(sp){
				for(int n=0;n<9;++n){
					buffin[n]=sp[5+n];
				}
				buffin[9]=0;
			}
			
			return ret;
		}else{
			fprintf(stderr,"Head not found\n");
			continue;
		}
		
	}
		
	return 0;
}
void gnuais(char *fileName)
{
	class dumpData d;
	char buff[2000];
	long int ifind;
	int ret;
	
	fprintf(stderr,"start gnuais\n");
	std::vector <list> listNames;

	listNames.clear();
	
	double start=rtime();
	double ticks=5;
	
	while(1){
		ret = nextBlockAis(buff,&d);	
		if(ret < 0){
			fprintf(stderr,"ret %d\n",ret);
			return;
		}
		//fprintf(stderr,"buff %s\n",buff);
		
		ifind=-1;
		
		if(ret != 0){
			for(unsigned long int n=0;n<listNames.size();++n){
				if(listNames[n].name == buff){
					ifind=(long)n;
				}
			}
			if(ifind >= 0 && d.NS != 0.0){
				//listNames[ifind].NS=d.NS;
				//listNames[ifind].EW=d.EW;
				int np=listNames[ifind].d.size()-1;
				listNames[ifind].d[np].NS=d.NS;
				listNames[ifind].d[np].EW=d.EW;
				continue;
			}
			if(d.Longitude == 0.0)continue;
			//fprintf(stderr,"listNames.size() %ld ifind %ld\n",(unsigned long)listNames.size(),(unsigned long)ifind);
			if(ifind >= 0){
				listNames[ifind].d.push_back(d);
				listNames[ifind].timeCreate=rtime();
			}else{	
				l.name=buff;
				l.d.clear();
				l.d.push_back(d);
				l.timeCreate=rtime();
				listNames.push_back(l);
			}
		}else{
			ticks = -1;		
		}
		
		double end=rtime();
		
		
		//fprintf(stderr,"end-start %g ticks %g \n",end-start,ticks);
				
		if((end-start) > ticks){
			fprintf(stderr,"ticks listNames.size() %ld\n",(long)listNames.size());
			FILE *out=fopen(fileName,"w");
			if(out){
				fprintf(out,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
				fprintf(out,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
				fprintf(out,"  <Document>\n");
			
			}
			for(unsigned long int n=0;n<listNames.size();++n){
				double diff=rtime() - listNames[n].timeCreate;
				if(diff > 3600)break;
				
				int np=listNames[n].d.size()-1;
				if(np < 0)np=0;				
				
				fprintf(stderr,"n %ld %s d.size() %ld diff %g heading %g\n",
				        n,listNames[n].name.c_str(),(long)listNames[n].d.size(),diff,listNames[n].d[np].heading);
				if(out && listNames[n].d.size() > 1){
					fprintf(out,"    <name>Flight Path</name>\n");
					fprintf(out,"       <Placemark>\n");
	
					fprintf(out,"          <Style>\n");
					fprintf(out,"            <LineStyle>\n");
	
					fprintf(out,"              <color>fff00fff</color>\n");
					fprintf(out,"              <width>2</width>\n");
	
					fprintf(out,"            </LineStyle>\n");
					fprintf(out,"          </Style>\n");
	
					fprintf(out,"          <name>%s</name>\n",listNames[n].name.c_str());
					fprintf(out,"          <LineString>\n");
					fprintf(out,"      <altitudeMode>relativeToGround</altitudeMode>\n");
					fprintf(out,"          <tessellate>1</tessellate>\n");
					fprintf(out,"          <coordinates>\n");
					
					for(unsigned long np=0;np<listNames[n].d.size();++np)
					{
						double longitude,latitude,height;		
		
						latitude=listNames[n].d[np].Latitude;
						longitude=listNames[n].d[np].Longitude;
						height=listNames[n].d[np].Altitude;
						fprintf(out,"             %f,%f,%f\n",longitude,latitude,height);
						
						
					}
					
					fprintf(out,"          </coordinates>\n");
					fprintf(out,"          </LineString>\n");
					fprintf(out,"       </Placemark>\n");
					
					{
					    int np=listNames[n].d.size()-1;
					    if(np < 0)break;
						//fprintf(stderr,"heading %g\n",listNames[n].d[np].heading);
						writeEndAis(out,listNames[n].d[np].heading,&listNames[n].d[np],n);
					}
					
			
				}
			}
			if(out){
				fprintf(out,"  </Document>\n");
				fprintf(out,"</kml>\n");
				fclose(out);
				out=NULL;
			}
			//fprintf(stderr,"File Closed\n");
			start=rtime();
		}
		if(ret <= 0)return;

	}	
	
	
}

void dump1090(char *fileName)
{
	class dumpData d;
	char buff[2000];
	long int ifind;
	int ret;
	
	fprintf(stderr,"start dump1090\n");
	
	
	std::vector <list> listNames;

	listNames.clear();
	
	double start=rtime();
	double ticks=5;
	
	while(1){
		ret = nextBlockDump(buff,&d);	
		if(ret < 0){
			fprintf(stderr,"ret %d\n",ret);
			return;
		}
		//fprintf(stderr,"%s %d\n",buff,(int)strlen(buff));
		ifind=-1;
		for(unsigned long int n=0;n<listNames.size();++n){
			if(listNames[n].name == buff){
				ifind=(long)n;
			}
		}
		if(ifind >= 0 && d.NS != 0.0){
			//listNames[ifind].NS=d.NS;
			//listNames[ifind].EW=d.EW;
			int np=listNames[ifind].d.size()-1;
			listNames[ifind].d[np].NS=d.NS;
			listNames[ifind].d[np].EW=d.EW;
			continue;
		}
		if(d.Longitude == 0.0)continue;
		//fprintf(stderr,"listNames.size() %ld ifind %ld\n",(unsigned long)listNames.size(),(unsigned long)ifind);
		if(ifind >= 0){
			listNames[ifind].d.push_back(d);
			listNames[ifind].timeCreate=rtime();
		}else{	
			l.name=buff;
			l.d.clear();
			l.d.push_back(d);
			l.timeCreate=rtime();
			listNames.push_back(l);
		}
		
		double end=rtime();
		
		if((end-start) > ticks){
			fprintf(stderr,"ticks listNames.size() %ld\n",(long)listNames.size());
			FILE *out=fopen(fileName,"w");
			if(out){
				fprintf(out,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
				fprintf(out,"<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
				fprintf(out,"  <Document>\n");
			
			}
			for(unsigned long int n=0;n<listNames.size();++n){
				double diff=rtime() - listNames[n].timeCreate;
				if(diff > 62)continue;
				fprintf(stderr,"n %ld %s d.size() %ld diff %g\n",n,listNames[n].name.c_str(),(long)listNames[n].d.size(),diff);
				if(out && listNames[n].d.size() > 4){
					fprintf(out,"    <name>Flight Path</name>\n");
					fprintf(out,"       <Placemark>\n");
	
					fprintf(out,"          <Style>\n");
					fprintf(out,"            <LineStyle>\n");
	
					fprintf(out,"              <color>fff00fff</color>\n");
					fprintf(out,"              <width>2</width>\n");
	
					fprintf(out,"            </LineStyle>\n");
					fprintf(out,"          </Style>\n");
	
					fprintf(out,"          <name>%s</name>\n",listNames[n].name.c_str());
					fprintf(out,"          <LineString>\n");
					fprintf(out,"      <altitudeMode>relativeToGround</altitudeMode>\n");
					fprintf(out,"          <tessellate>1</tessellate>\n");
					fprintf(out,"          <coordinates>\n");
					
					double NS=0.0;
					double EW=0.0;
					for(unsigned long np=0;np<listNames[n].d.size();++np)
					{
						double longitude,latitude,height;		
		
						latitude=listNames[n].d[np].Latitude;
						longitude=listNames[n].d[np].Longitude;
						height=listNames[n].d[np].Altitude;
						fprintf(out,"             %f,%f,%f\n",longitude,latitude,height);
						
						if(listNames[n].d[np].NS != 0.0)NS=listNames[n].d[np].NS;
						if(listNames[n].d[np].EW != 0.0)EW=listNames[n].d[np].EW;
						
					}
					
					fprintf(out,"          </coordinates>\n");
					fprintf(out,"          </LineString>\n");
					fprintf(out,"       </Placemark>\n");
					
					if(NS || EW){
					    int np=listNames[n].d.size()-1;
					    if(np < 0)continue;
						//fprintf(stderr,"NS %g EW %g\n",NS,EW);
						writeEndDump(out,NS,EW,&listNames[n].d[np],n);
					}
			
				}
			}
			if(out){
				fprintf(out,"  </Document>\n");
				fprintf(out,"</kml>\n");
				fclose(out);
				out=NULL;
			}
			start=rtime();
		}

	}
	
	fprintf(stderr,"end dump1090 \n");
}
int writeEndDump(FILE *out,double NS,double EW,class dumpData *d,int n)
{
	double latitude,longitude,height;
	struct P pc,pup,px,pz,up;
	struct System p1;
	struct System p2;
	struct Matrix mf,mb;

	latitude = (d->Latitude);
	longitude= (d->Longitude);
	height=d->Altitude;
	pc=Wgs84Forward(latitude,longitude,0.0);
	
	pup=Wgs84Forward(latitude,longitude,10.0);
	pup=Sub(&pup,&pc);
	pup=Norm(&pup);
	
	px=Wgs84Forward(latitude+1e-6,longitude,0.0);
	px=Sub(&px,&pc);
	px=Norm(&px);
	
	pz=Wgs84Forward(latitude,longitude+1e-6,0.0);
	pz=Sub(&pz,&pc);
	pz=Norm(&pz);
	
	up=CrossN(&pz,&px);
		
	p1.x=p(1,0,0);
	p1.y=p(0,1,0);
	p1.z=p(0,0,1);
	p1.p=p(0,0,0);
	
	p2.x=pz;
	p2.y=px;
	p2.z=up;
	p2.p=pc;
	
	BuildTransformMatrix(&mf,&p1,&p2);
	
	BuildTransformMatrix(&mb,&p2,&p1);

	double xc,yc,zc;
	double x,y,z;
	unsigned char rr,gg,bb,aa;
	struct P pg,pl;


	pg=Wgs84Forward(latitude,longitude,height);
	
	pl=Transform(&mf,&pg);
	
	xc=pl.x;
	yc=pl.y;
	zc=pl.z;
	
	rr=0;
	gg=0xff;
	bb=0;
	aa=(unsigned char)(0xff);
	
	fprintf(out,"    <Placemark>\n");
	
	fprintf(out,"      <Style><PolyStyle><outline>0</outline><fill>1</fill>");
	fprintf(out,"<color>%02x%02x%02x%02x</color></PolyStyle></Style>\n",aa,bb,gg,rr);
	
	char buff[512];

	snprintf(buff,sizeof(buff),"path %d",n);
	
	fprintf(out,"      <name>%s</name>\n",buff);
	
	fprintf(out,"      <Polygon>\n");
	
	fprintf(out,"      <altitudeMode>relativeToGround</altitudeMode>\n");

	
	//fprintf(out,"      <tessellate>1</tessellate>\n");
				
	fprintf(out,"      <outerBoundaryIs><LinearRing><coordinates>\n");
	
	
	fprintf(out,"        ");
	int writeMarkerType=0;
	double writeMarkerSize=200;
	if(writeMarkerType == 0)
	{
		struct P pb,pp[5];
		
		for(int j=0;j<5;++j)
		{
			x=zc;
			y=zc;
			z=zc;
			switch(j)
			{
				case 0:
				case 4:
					x=xc-1.5*writeMarkerSize;
					y=yc-2.5*writeMarkerSize;
					break;
				case 1:
					x=xc+1.5*writeMarkerSize;
					y=yc-2.5*writeMarkerSize;
					break;
				case 2:
					//x=xc+0.5*writeMarkerSize;
					x=xc;
					y=yc+2.5*writeMarkerSize;
					break;
				case 3:
					//x=xc-0.5*writeMarkerSize;
					x=xc;
					y=yc+2.5*writeMarkerSize;
					break;
			}
			pp[j]=p(x,y,z);
		}
	
		for(int j=0;j<5;++j)
		{
	
			double theta1=atan2(NS,EW);
			double rot=theta1-M_PI/2.0;
			double theta2=atan2(pp[j].y,pp[j].x);
			double r=sqrt(pp[j].y*pp[j].y+pp[j].x*pp[j].x);
			pb=pp[j];
			pb.x=xc+r*cos(theta2+rot);
			pb.y=yc+r*sin(theta2+rot);
			pb=Transform(&mb,&pb);
			pb=Wgs84Reverse(pb.x,pb.y,pb.z);
			fprintf(out,"%f,%f,%f ",pb.y,pb.x,pb.z);

		}
	
		fprintf(out,"\n");
	}else{
		;
	}
	
	fprintf(out,"      </coordinates></LinearRing></outerBoundaryIs>\n");
	fprintf(out,"      </Polygon>\n");
	fprintf(out,"    </Placemark>\n");
	
	
	return 0;
}

int writeEndAis(FILE *out,double heading,class dumpData *d,int n)
{
	double latitude,longitude,height;
	struct P pc,pup,px,pz,up;
	struct System p1;
	struct System p2;
	struct Matrix mf,mb;

	latitude = (d->Latitude);
	longitude= (d->Longitude);
	height=d->Altitude;
	pc=Wgs84Forward(latitude,longitude,0.0);
	
	pup=Wgs84Forward(latitude,longitude,10.0);
	pup=Sub(&pup,&pc);
	pup=Norm(&pup);
	
	px=Wgs84Forward(latitude+1e-6,longitude,0.0);
	px=Sub(&px,&pc);
	px=Norm(&px);
	
	pz=Wgs84Forward(latitude,longitude+1e-6,0.0);
	pz=Sub(&pz,&pc);
	pz=Norm(&pz);
	
	up=CrossN(&pz,&px);
		
	p1.x=p(1,0,0);
	p1.y=p(0,1,0);
	p1.z=p(0,0,1);
	p1.p=p(0,0,0);
	
	p2.x=pz;
	p2.y=px;
	p2.z=up;
	p2.p=pc;
	
	BuildTransformMatrix(&mf,&p1,&p2);
	
	BuildTransformMatrix(&mb,&p2,&p1);

	double xc,yc,zc;
	double x,y,z;
	unsigned char rr,gg,bb,aa;
	struct P pg,pl;


	pg=Wgs84Forward(latitude,longitude,height);
	
	pl=Transform(&mf,&pg);
	
	xc=pl.x;
	yc=pl.y;
	zc=pl.z;
	
	rr=0;
	gg=0xff;
	bb=0;
	aa=(unsigned char)(0x7f);
	
	fprintf(out,"    <Placemark>\n");
	
	fprintf(out,"      <Style><PolyStyle><outline>0</outline><fill>1</fill>");
	fprintf(out,"<color>%02x%02x%02x%02x</color></PolyStyle></Style>\n",aa,bb,gg,rr);
	
	char buff[512];

	snprintf(buff,sizeof(buff),"path %d",n);
	
	fprintf(out,"      <name>%s</name>\n",buff);
	
	fprintf(out,"      <Polygon>\n");
	
	fprintf(out,"      <altitudeMode>relativeToGround</altitudeMode>\n");

	
	//fprintf(out,"      <tessellate>1</tessellate>\n");
				
	fprintf(out,"      <outerBoundaryIs><LinearRing><coordinates>\n");
	
	
	fprintf(out,"        ");
	int writeMarkerType=0;
	double writeMarkerSize=200;
	if(writeMarkerType == 0)
	{
		struct P pb,pp[5];
		
		for(int j=0;j<5;++j)
		{
			x=zc;
			y=zc;
			z=zc;
			switch(j)
			{
				case 0:
				case 4:
					x=xc-1.5*writeMarkerSize;
					y=yc-2.5*writeMarkerSize;
					break;
				case 1:
					x=xc+1.5*writeMarkerSize;
					y=yc-2.5*writeMarkerSize;
					break;
				case 2:
					//x=xc+0.5*writeMarkerSize;
					x=xc;
					y=yc+2.5*writeMarkerSize;
					break;
				case 3:
					//x=xc-0.5*writeMarkerSize;
					x=xc;
					y=yc+2.5*writeMarkerSize;
					break;
			}
			pp[j]=p(x,y,z);
		}
	
		for(int j=0;j<5;++j)
		{
	
			double theta1=heading*M_PI/180.0;
			double rot=theta1-M_PI/2.0;
			double theta2=atan2(pp[j].y,pp[j].x);
			double r=sqrt(pp[j].y*pp[j].y+pp[j].x*pp[j].x);
			pb=pp[j];
			pb.x=xc+r*cos(theta2+rot);
			pb.y=yc+r*sin(theta2+rot);
			pb=Transform(&mb,&pb);
			pb=Wgs84Reverse(pb.x,pb.y,pb.z);
			fprintf(out,"%f,%f,%f ",pb.y,pb.x,pb.z);

		}
	
		fprintf(out,"\n");
	}else{
		;
	}
	
	fprintf(out,"      </coordinates></LinearRing></outerBoundaryIs>\n");
	fprintf(out,"      </Polygon>\n");
	fprintf(out,"    </Placemark>\n");
	
	
	return 0;
}

int nextLineAis(char *buff,int *nn)
{
	char buff1[10];
	int ret;
	int c;
	int n;
	
	*nn=0;
	
	n=0;
	while(1){
		ret = (int)fread(buff1,1,1,stdin);	
		if(ret <= 0)return ret;
		c=buff1[0];
		if(c == '\n' || c == '\r'){
			buff[n]=0;
			*nn = n;
			return n;
		} else if(n < 1999){
			buff[n++]=c;
		}else{
			*nn = n;
			return n;
		}
	}
}
int nextLineDump(char *buff,int *nn)
{
	char buff1[10];
	int ret;
	int c;
	int n;
	
	*nn=0;
	
	n=0;
	while(1){
		ret = (int)fread(buff1,1,1,stdin);	
		if(ret < 0)return ret;
		c=buff1[0];
		if(c == '\n' || c == '\r'){
			buff[n]=0;
			*nn = n;
			return n;
		} else if(n < 1999){
			buff[n++]=c;
		}else{
			*nn = n;
			return n;
		}
	}
}
int nextBlockDump(char *buffin,class dumpData *d)
{
	char buff[2000];
	char ICAO[]={"ICAO Address"};
	char Alt[]={"Altitude"};
	char Lat[]={"Latitude"};
	char Long[]={"Longitude"};
	char NS[]={"NS velocity"};
	char EW[]={"EW velocity"};
	int ret;
	int nn;

	
	d->Altitude=0;
	d->Latitude=0;
	d->Longitude=0;
	d->NS=0;
	d->EW=0;
		
	while(1){
		ret = nextLineDump(buff,&nn);	
		if(ret < 0){
			fprintf(stderr,"ret %d\n",ret);
			return 1;
		}
		if(*buff == '*')break;
	}
	while(1){
		ret = nextLineDump(buff,&nn);	
		if(ret < 0){
			fprintf(stderr,"ret %d\n",ret);
			return 1;
		}
		
		//fprintf(stderr,"%s\n",buff);
		
		if(ret == 0)return 0;
		
		if(nn >= 18 && buff[4] == NS[0] && buff[5] == NS[1]){
			char *sp;			
			sp=strstr(buff,"Valid");
			if(sp)continue;
		    int ip=-1;
		    for(int n=2;n<nn;++n){
		    	if(buff[n] == ':'){
		    		ip=n;
		    		break;
		    	}
		    } 
		    //fprintf(stderr,"",ip);
		    ip += 2;
			if(ip > 0){				
			 	d->NS=atof(&buff[ip]);
			   // fprintf(stderr,"NS=%f\n",d->NS);			 	
			 	continue;
			 }
		}
		
		
		
		if(nn >= 18 && buff[4] == EW[0] && buff[5] == EW[1]){
			char *sp;			
			sp=strstr(buff,"Valid");
			if(sp)continue;
		    int ip=-1;
		    for(int n=2;n<nn;++n){
		    	if(buff[n] == ':'){
		    		ip=n;
		    		break;
		    	}
		    } 
		    //fprintf(stderr,"",ip);
		    ip += 2;
			if(ip > 0){				
			 	d->EW=atof(&buff[ip]);
			    //fprintf(stderr,"EW=%f\n",d->EW);			 	
			 	continue;
			 }
		}
		
		
		
		
		if(nn >= 18 && buff[2] == Alt[0] && buff[3] == Alt[1]){
		    int ip=-1;
		    for(int n=2;n<nn;++n){
		    	if(buff[n] == ':'){
		    		ip=n;
		    		break;
		    	}
		    } 
		    //fprintf(stderr,"",ip);
		    ip += 2;
			if(ip > 0){				
			    char *sp;			
			    sp=strstr(buff,"not");
			    if(sp)continue;			 	
			 	d->Altitude=atof(&buff[ip]);
			    //fprintf(stderr,"Altitude=%f\n",d->Altitude);			 	
			    sp=strstr(buff,"feet");
			    if(sp)d->Altitude=d->Altitude*0.3048;			 	
			 	continue;
			 }
		}
		
		if(nn >= 18 && buff[4] == Alt[0] && buff[5] == Alt[1]){
		    int ip=-1;
		    for(int n=2;n<nn;++n){
		    	if(buff[n] == ':'){
		    		ip=n;
		    		break;
		    	}
		    } 
		    //fprintf(stderr,"",ip);
		    ip += 2;
			if(ip > 0){				
			    char *sp;			
			    sp=strstr(buff,"not");
			    if(sp)continue;			 	
			 	d->Altitude=atof(&buff[ip]);
			    //fprintf(stderr,"Altitude=%f\n",d->Altitude);			 	
			    sp=strstr(buff,"feet");
			    if(sp)d->Altitude=d->Altitude*0.3048;			 	
			 	continue;
			 }
		}
		
		if(nn >= 18 && buff[4] == Lat[0] && buff[5] == Lat[1]){
		    int ip=-1;
		    for(int n=2;n<nn;++n){
		    	if(buff[n] == ':'){
		    		ip=n;
		    		break;
		    	}
		    } 
		    //fprintf(stderr,"",ip);
		    ip += 2;
			if(ip > 0){				
			    char *sp;			
			    sp=strstr(buff,"not");
			    if(sp)continue;			 	
			 	d->Latitude=atof(&buff[ip]);
			 	continue;
			 }
		}
		
		if(nn >= 18 && buff[4] == Long[0] && buff[5] == Long[1]){
		    int ip=-1;
		    for(int n=2;n<nn;++n){
		    	if(buff[n] == ':'){
		    		ip=n;
		    		break;
		    	}
		    } 
		    //fprintf(stderr,"",ip);
		    ip += 2;
			if(ip > 0){	
			    char *sp;			
			    sp=strstr(buff,"not");
			    if(sp)continue;		
			 	d->Longitude=atof(&buff[ip]);
			 	continue;
			 }
		}
		
		
		
		
		if(nn >= 22 && buff[2] == ICAO[0] && buff[3] == ICAO[1]){
		    int ip=-1;
		    for(int n=2;n<nn;++n){
		    	if(buff[n] == ':'){
		    		ip=n;
		    		break;
		    	}
		    } 
		    //fprintf(stderr,"",ip);
		    ip += 2;
			if(ip > 0){				
			 //	fprintf(stderr,"ICAO=%s\n",&buff[ip]);
			 	strncpy(buffin,&buff[ip],6);
			 }
		}
	}
	return 0;
}

int GetTime(long *Seconds,long *milliseconds)
{
	struct timeb t;
	
	if(!Seconds || !milliseconds)return 1;
	

	ftime(&t);

	*Seconds=(long)t.time;
	*milliseconds=t.millitm;
	
	return 0;
}
double rtime(void)
{
	long milliseconds;
	long Seconds;
	double ret;
	
	
	GetTime(&Seconds,&milliseconds);
	
	ret=(double)Seconds+(double)milliseconds/1000.;
	
	return ret;

}
struct P Norm(struct P *p1)
{
    struct P p;
    double sum;

    sum=1./pow(p1->x*p1->x+p1->y*p1->y+p1->z*p1->z,.5);
    p.x=(p1->x*sum);
    p.y=(p1->y*sum);
    p.z=(p1->z*sum);
    return p;
}
struct P CrossN(struct P *v1,struct P *v2)
{
    static struct P Zero;
    struct P Vcross;
    double Length;

    Vcross.x =   v1->y*v2->z-v2->y*v1->z;
    Vcross.y = -(v1->x*v2->z-v2->x*v1->z);
    Vcross.z =   v1->x*v2->y-v2->x*v1->y;
    Length=Len(&Vcross);
    if(Length <= 0.)return Zero;
    Vcross.x/=Length;
    Vcross.y/=Length;
    Vcross.z/=Length;

    return  Vcross;
}
struct P Sub(struct P *p1,struct P *p2)
{
    struct P psub;

    psub.x=p1->x-p2->x;
    psub.y=p1->y-p2->y;
    psub.z=p1->z-p2->z;
    return psub;
 
}
struct P Add(struct P *p1,struct P *p2)
{
    struct P padd;

    padd.x=p1->x+p2->x;
    padd.y=p1->y+p2->y;
    padd.z=p1->z+p2->z;
    return padd;
 
}
struct P Mult(struct P *p1,double Factor)
{
    struct P p;

    p.x=(p1->x*Factor);
    p.y=(p1->y*Factor);
    p.z=(p1->z*Factor);
    return p;
}
double Len(struct P *v1)
{
    double Lenr;

    Lenr=pow((v1->x)*(v1->x)+
            (v1->y)*(v1->y)+
            (v1->z)*(v1->z),.5);

    return  Lenr;
}
double Dot(struct P *v1,struct P *v2)
{
    double Length;

    Length=(v1->x*v2->x)+(v1->y*v2->y)+(v1->z*v2->z);

    return  Length;
}
struct P p(double x,double y,double z)
{
	struct P ret;
	
	ret.x=x;
	ret.y=y;
	ret.z=z;
	
	return ret;
}

Flt VecNormalize(Vec vec)
{
	Flt len ;
	len = (Flt) VecLen(vec);
	if(len > 0.0){
		vec[0]/=len ;
		vec[1]/=len ;
		vec[2]/=len ;
	}
	return(len) ;
}
void BuildTransformMatrix(struct Matrix *m,struct System *p1,struct System *p2)
{
    /*   Matrix to transform a point from system p1 to system p2  */

	struct P p12;
	
	if(!m || !p1 || !p2)return;
	
	m->x[0][0]=Dot(&p2->x,&p1->x);
	m->x[1][0]=Dot(&p2->y,&p1->x);
	m->x[2][0]=Dot(&p2->z,&p1->x);
	m->x[3][0]=0;
	
	m->x[0][1]=Dot(&p2->x,&p1->y);
	m->x[1][1]=Dot(&p2->y,&p1->y);
	m->x[2][1]=Dot(&p2->z,&p1->y);
	m->x[3][1]=0;
	
	m->x[0][2]=Dot(&p2->x,&p1->z);
	m->x[1][2]=Dot(&p2->y,&p1->z);
	m->x[2][2]=Dot(&p2->z,&p1->z);
	m->x[3][2]=0;
	
	p12=Sub(&p1->p,&p2->p);
	
	m->x[0][3]=Dot(&p2->x,&p12);
	m->x[1][3]=Dot(&p2->y,&p12);
	m->x[2][3]=Dot(&p2->z,&p12);
	m->x[3][3]=1;
}

int Wgs84BuildLocalSystem(double latitude,double longitude,double height,struct Matrix *mf,struct Matrix *mb)
{
	struct P pc,pup,px,pz,up;
	struct System p1;
	struct System p2;
	
	if(!mf || !mb)return 1;
	
	pc=Wgs84Forward(latitude,longitude,height);
	
	pup=Wgs84Forward(latitude,longitude,height+10.0);
	pup=Sub(&pup,&pc);
	pup=Norm(&pup);
	
	px=Wgs84Forward(latitude+1e-6,longitude,height);
	px=Sub(&px,&pc);
	px=Norm(&px);
	
	pz=Wgs84Forward(latitude,longitude+1e-6,height);
	pz=Sub(&pz,&pc);
	pz=Norm(&pz);
	
	up=CrossN(&pz,&px);
	
	p1.x=p(1,0,0);
	p1.y=p(0,1,0);
	p1.z=p(0,0,1);
	p1.p=p(0,0,0);
	
	p2.x=pz;
	p2.y=px;
	p2.z=up;
	p2.p=pc;
	
	BuildTransformMatrix(mf,&p1,&p2);
	
	BuildTransformMatrix(mb,&p2,&p1);
	
	return 0;
}
struct P Wgs84Forward(double latitude,double longitude,double height)
{
	double nt,a,b,f,of,e2,sint,cost,dtor;
	struct P p;
	
	dtor=atan(1.0)/45.;
	a=6378137.0;
	of=298.257223563;
	f=1.0/of;
	b=a*(1.0-f);
	e2=1.0-(b*b)/(a*a);
	sint=sin(latitude*dtor);
	cost=cos(latitude*dtor);
	nt=a/sqrt(1.0-e2*sint*sint);
	
	p.x=(nt+height)*cost*cos(longitude*dtor);
	p.y=(nt+height)*cost*sin(longitude*dtor);
	p.z=(nt*(1.0-e2)+height)*sint;
	
	return p;
	
}
struct P Wgs84Reverse(double x,double y,double z)
{
	double B;
	double d;
	double e;
	double f;
	double g;
	double p;
	double q;
	double r;
	double t;
	double v;
	double zlong;
	double twopi=8.0*atan(1.0);
	double rad_to_deg=360.0/twopi;
	double A=6378137.0;
	double of=298.257223563;
	double FL=1.0/of;
	struct P ret;
	double plh[3];
	
	/*
	 *   1.0 compute semi-minor axis and set sign to that of z in order
	 *       to get sign of Phi correct
	 */
	B= A * (1.0 - FL);
	if( z < 0.0 )
		B= -B;
	/*
	 *   2.0 compute intermediate values for latitude
	 */
	r= sqrt( x*x + y*y );
	e= ( B*z - (A*A - B*B) ) / ( A*r );
	f= ( B*z + (A*A - B*B) ) / ( A*r );
	/*
	 *   3.0 find solution to:
	 *       t^4 + 2*E*t^3 + 2*F*t - 1 = 0
	 */
	p= (4.0 / 3.0) * (e*f + 1.0);
	q= 2.0 * (e*e - f*f);
	d= p*p*p + q*q;
	
	if( d >=  0.0 ) {
		v= pow( (sqrt( d ) - q), (1.0 / 3.0) )
		- pow( (sqrt( d ) + q), (1.0 / 3.0) );
	} else {
		v= 2.0 * sqrt( -p )
		* cos( acos( q/(p * sqrt( -p )) ) / 3.0 );
	}
	/*
	 *   4.0 improve v
	 *       NOTE: not really necessary unless point is near pole
	 */
	if( v*v < fabs(p) ) {
		v= -(v*v*v + 2.0*q) / (3.0*p);
	}
	g= (sqrt( e*e + v ) + e) / 2.0;
	t = sqrt( g*g  + (f - v*g)/(2.0*g - e) ) - g;
	
	plh[0] = atan( (A*(1.0 - t*t)) / (2.0*B*t) );
	/*
	 *   5.0 compute height above ellipsoid
	 */
	plh[2]= (r - A*t)*cos( plh[0] ) + (z - B)*sin( plh[0] );
	/*
	 *   6.0 compute longitude east of Greenwich
	 */
	
	zlong = atan2( y, x );
	
	/*if( zlong < ZERO )
	 zlong= zlong + twopi; */
	
	plh[1]= zlong;
	/*
	 *   7.0 convert latitude and longitude to degrees
	 */
	plh[0] = plh[0] * rad_to_deg;
	plh[1] = plh[1] * rad_to_deg;
	
	ret.x=plh[0];
	ret.y=plh[1];
	ret.z=plh[2];
	
	
	return ret;
}
struct P Transform(struct Matrix *m,struct P *pc)
{
	double v1x,v1y,v1z,v2x,v2y,v2z,v3x,v3y,v3z;
	double xc,yc,zc,dx,dy,dz;

	struct P ret;

	v1x=m->x[0][0];
	v1y=m->x[0][1];
	v1z=m->x[0][2];

	v2x=m->x[1][0];
	v2y=m->x[1][1];
	v2z=m->x[1][2];
	
	v3x=m->x[2][0];
	v3y=m->x[2][1];
	v3z=m->x[2][2];
	
	xc=m->x[0][3];
	yc=m->x[1][3];
	zc=m->x[2][3];
	
	
	dx=pc->x;
	dy=pc->y;
	dz=pc->z;
	ret.x=((dx)*v1x+(dy)*v1y+(dz)*v1z+xc);
	ret.y=((dx)*v2x+(dy)*v2y+(dz)*v2z+yc);
	ret.z=((dx)*v3x+(dy)*v3y+(dz)*v3z+zc);

	return ret;
}
