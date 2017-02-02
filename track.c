#include <stdio.h>
#include <math.h>
#include <time.h>
#include "sat.h"
#include "tft.h"

#define KBSIGNAL    2001

#define ALL         0
#define TRACK       1
#define MENU        2
#define QUIT        9

#define TEXT        0
#define MERCATOR    1
#define SATVIEW     2

#define UPKEY       2
#define DOWNKEY     4
#define SELECTKEY   8
#define BACKKEY     16

#define Read_   1
#define Write_  2

int stepsiz[] = {50, 50, 50};
int step[3];

SATELLITE *sat[MAXSATS];
int sort[MAXSATS];
int numsat, selectedSat;

char c, str[64];
int mode, viewmode;
double sx, sy, sz, az, el, m256, r, vv, alt, vor, ror, fpa;
double olat, olon, slat, slon;
double dop;
int ohgt, rn;
int zoom, autozoom, showBorder, showGrid;
int n = 0;
struct tm *tp;

SATELLITE *readTLE();

char getKey()
{
    char *inport = 0x80001a;
    return (~*inport);
}

printHeader()
{
    setBgColor(BLACK);
    setFgColor(MAGENTA);
    cursor(0,0);
    sprintf(str, "satellite          az    el   height     range\n");
    prstr(str);
    sprintf(str, "------------------------------------------------\n");
    prstr(str);

}

printDateTime()
{
    setBgColor(BLACK);
    setFgColor(MAGENTA);

    cursor(16,34);
    sprintf(str, "%02d:%02d:%02d (UTC)",
        tp->tm_hour, tp->tm_min, tp->tm_sec);
    prstr(str);
}

printSat(i, row, highlight)
int i, row, highlight;
{
    setBgColor(BLACK);
    setFgColor(CYAN);

    if (i<0) {
        sprintf(str, "                                                ");
    }
    else {
        if (sat[i]->el>0)
            setFgColor(YELLOW);
        if (highlight)
            setBgColor(BLUE);
        sprintf(str, "%-15s %5.1f %5.1f  %5.0fkm   %5.0fkm  \n",
            sat[i]->name, sat[i]->az, sat[i]->el, sat[i]->ror, sat[i]->r);
    }

    cursor(row+3, 0);
    prstr(str);
    setBgColor(BLACK);
}

sortSat(i)
int i;
{
    int j, k;
    
    for (j=0; j<numsat; j++) {
        if (sat[i]->el>=sat[sort[j]]->el) {
            for (k=j; k<numsat; k++) {
                if (sort[k]==i) break;
            }
            for ( ; k>j; k--) {
                sort[k]=sort[k-1];
            }
            sort[j]=i;
            break;
        }
    }
}

refresh(first, row)
int first;
{
    int i, j;
    
    /* fast update of screen */ 
    for (i=0; i<12; i++) {
        j=-1;
        if (first+i<numsat) {
            j=sort[first+i];
        }
        printSat(j, i, i==row);
    }
}

readFreq(filename)
char *filename;
{
    FILE *fp;
    int i, j;
    char *lp, name[16], line[64];
    
    fp=fopen(filename,"r");

    for (;;) {

        fgets(name,16,fp);
        i = strlen(name);
        if (i==0) break;
        name[i-1]=0;
        
        /* search satellite in sat[] */             
        for (i=0; i<numsat; i++) {
            
            if (!strcmp(sat[i]->name, name)) {

                /* ok, sat found, now read frequencies (max 3)*/
                for (j=0; j<2; j++) {
                    
                    /* read freq line, format <freq> <description> */
                    fgets(line,64,fp);
                    if (strlen(line)<2) continue;
                    lp = line;
                    sat[i]->link[j].freq=atof(line);
                    while (*lp++>' ');
                    strcpy(sat[i]->link[j].desc, lp);
                }
                break;
            }
        }
    }
}
        
getTime()
{
    time_t timer;
    
    timer = time(0);
    timer -= 3600;
    tp = localtime(&timer);
    tp->tm_year -= 100;
    tp->tm_mon += 1;
}

int showAllSats()
{
    int i, j;
    int k=0;
    int start=0;
    char c;

    cls();
    printHeader();
/*  
    while (selectedSat != sort[start+k]) {
        if (++k==12) {
            k=0; 
            start += 12;
        }
    }
*/
    refresh(start, k);

    for (;;) {
        
        getTime();

        cursor(0,0);
        for (i=0; i<12; i++) {
            j=-1;
            if (start+i<numsat) {
                j = sort[start+i];
                getTime();
                satvec(sat[j], tp);
                c = getKey();
                if (c>0) break;
                printDateTime();
                printSat(j, i, i==k);
                sortSat(j);
            }
        }

        satvec(sat[n], tp);
        sortSat(n);
        if (++n>=numsat) n=0;

        c = getKey();
        if (c>0) {
            switch (c) {
                case DOWNKEY: 
                    printSat(sort[start+k], k, FALSE);
                    k++;
                    if (k>11) {
                        start+=12;
                        k=0;
                        if (start>numsat-1) {
                            start-=12;
                            k=11;
                        }
                        refresh(start, k);
                    }
                    printSat(sort[start+k], k, TRUE);
                    break;
                case UPKEY:
                    printSat(sort[start+k], k, FALSE);
                    k--;
                    if (k<0) {
                        start-=12;
                        k=11;
                        if (start<0) {
                            start=0;
                            k=0;
                        }
                        refresh(start, k);
                    }
                    printSat(sort[start+k], k, TRUE);
                    break;
                case SELECTKEY:
                    selectedSat = sort[start+k];
                    return TRACK;
                case BACKKEY:
                    return QUIT;
            }
        }
        
    
    }
}

int trackSat()
{
    char c;
    
    cls();
    
    for (;;) {
        
        getTime();

        satvec(sat[n], tp);
        sortSat(n);
        if (++n>=numsat) n=0;

        satvec(sat[selectedSat], tp);
        dop = -vv/299792;
        doppler();

        c = getKey();
        if (c>0) {
            cls();
            switch (c) {
                case SELECTKEY:
                    if(++viewmode>SATVIEW) viewmode=TEXT;
                    break;
                case UPKEY:
                    if (autozoom) {
                        zoom = 0;
                        autozoom = FALSE;
                    }
                    zoom++;
                    if (zoom>64) zoom=64;
                    break;
                case DOWNKEY:
                    zoom--;
                    if (zoom==0) {
                        zoom = 1;
                        autozoom = TRUE;
                    }
                    break;
                case BACKKEY:
                    return ALL;
            }
        }
        
        switch (viewmode) {
            case TEXT:
                printData(selectedSat);
                break;
            case MERCATOR:
                plotMercatorView(sx, sy, sz);
                printDataLine(selectedSat);
                break;
            case SATVIEW:
                plotSatView(sx, sy, sz);
                printDataLine(selectedSat);
                break;
        }               
    }
}

doppler()
{
    char *set = 0x80001c;
    char *clr = 0x80001e;
    int stctr, stlim, stsw, j;
    double f, dpl;

    for (j=0; j<2; j++) {
            
        f = sat[selectedSat]->link[j].freq;
        dpl = 1000*f*dop;
            
        if (f>0) {
            stctr = 1000/stepsiz[j];
            stlim = 300*stctr;

            do {
                stsw = (int)dpl/stepsiz[j] - step[j];
                if (stsw==0)
                    break;
                    
                if (step[j]>stlim)
                    step[j] = 0;

                if (stsw>0) {
                    step[j]++;
                    *set = 4<<j<<j;
                    tsleep(1);
                    *clr = 4<<j<<j;
                    tsleep(1);
                }
                else {
                    step[j]--;
                    *set = 8<<j<<j;
                    tsleep(1);
                    *clr = 8<<j<<j;
                    tsleep(1);
                }
            } while (--stctr > 0);
        }
    }
}

printData(i)
int i;
{
    int j;
    
    cursor(0,0);

    setFgColor(MAGENTA);
    sprintf(str, "%-11s             %02d/%02d/%04d  %02d:%02d:%02d\n\n", 
        sat[i]->name,
        tp->tm_mday, tp->tm_mon, 2000+tp->tm_year,
        tp->tm_hour, tp->tm_min, tp->tm_sec);
    prstr(str);

    setFgColor(YELLOW); 
    sprintf(str, "Azimuth    %5.1f deg    Longitude  %5.1f deg\n", az, slon);
    prstr(str);
    sprintf(str, "Elevation  %5.1f deg    Latitude   %5.1f deg\n\n", el, slat);
    prstr(str);

    setFgColor(CYAN);
    sprintf(str, "MA         %5.1f        Orbit nr.  %d\n", m256, rn);
    prstr(str);
    sprintf(str, "Distance   %5.0f km     Rel. speed %5.1f km/s\n", r, vv);
    prstr(str);
    sprintf(str, "Altitude %7.0f km     Radius   %7.0f km\n", alt, ror);
    prstr(str);
    sprintf(str, "Velocity %7.1f km/s   Direction  %5.1f deg\n\n", vor, fpa);
    prstr(str);

    for (j=0; j<2; j++) {
        if (sat[i]->link[j].freq==0)
            sprintf(str, "Link %d: -\n");
        else
            sprintf(str, "Link %d: %10.3f (%6.3f) %s",
                j, sat[i]->link[j].freq*(1+dop),
                sat[i]->link[j].freq*dop, sat[i]->link[j].desc);
        prstr(str);
    }
}

printDataLine(i)
int i;
{
    setFgColor(CYAN);
    cursor(16,0);
    sprintf(str, "%-11s %5.1f %5.1f  %5.0fkm    %02d:%02d:%02d UTC", 
        sat[i]->name, az, el, r,    
        tp->tm_hour, tp->tm_min, tp->tm_sec);
    prstr(str);
}

main()
{
    FILE *fp;
    int i;

    tft_init();
    cls();
    
    setBgColor(BLACK);
    setFgColor(CYAN);
    
    sprintf(str, "Initializing..\n\n");
    prstr(str);

    olon = 5.2;
    olat = 52.2;
    ohgt = 24;

    setObserver(olon, olat, ohgt);
    
    initView();
    
    fp = fopen("kepler.dat", "r");
    for (i=0; i<MAXSATS; i++) {
        sat[i] = readTLE(fp);
        if (sat[i] == 0)
            break;
        sort[i] = i;
    }
    fclose(fp);

    numsat = i; 
    sprintf(str, "%d satellites read\n", numsat);
    prstr(str);
    sleep(1);
    
    readFreq("freq.dat");

    selectedSat = 0;
    
    mode = ALL;
    viewmode = TEXT;
    
    for (;;) {
        switch (mode) {
            case ALL:
                mode = showAllSats();
                break;
            case TRACK:
                mode = trackSat();
                break;
/*          case MENU:
                mode = menu();
                break; */
            case QUIT:
                exit(0);
        }
    }
}
