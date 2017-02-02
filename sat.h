#define FALSE   0
#define TRUE    !FALSE

#define G0      75369793000000.0        /* GM of earth in (orbits/day)^2/km^3 */
#define G1      1.0027379093            /* Siderial/Solar time */
#define MAXSATS 200

static double RE = 6378.137;
static double RE2 = 40680631.590769;    /* earth radius squared */
static double FL = 1.0/298.257224;
static double GM = 3.986E5;
static double J2 = 1.08263E-3;
static double YM = 365.25;
static double YT = 365.2421874;
static double WW = 2.0*PI/365.2421874;
static double WE = 2.0*PI*(1+1/365.2421874) ;
static double YG = 2000.;
/*static double G0 = 98.9821;*/
static double MAS0 = 356.0507;
static double MASD = 0.98560028;
static double EQC1 = 0.03342 ;
static double EQC2 = 0.00035 ;
static double INS = 23.4393*PI/180.0 ;
static double CNS = 0.917482000; /*cos(23.4393*PI/180.0) ;*/
static double SNS = 0.397777298; /*sin(23.4393*PI/180.0) ;*/

#define deg(x)  (x/PI*180.0)
#define rad(x)  (x/180.0*PI)
#define gmst(x) ((99.6367-0.2387*(x-1989)+0.9856*(int)((x-1989)/4.0))/360.0)

typedef struct {
    double freq;
    char desc[32];
} LINK;

typedef struct {
    char   name[25];
    long   catnum;
    int    ey;              /* epoch year */
    double ed;              /* epoch julian day */
    double orbitnum;
    double in;              /* inclination */
    double ra;
    double ec;              /* eccentricity */
    double wp;              /* arg of perigee */
    double ma;
    double mm;              /* mean motion */
    int    rv;              /* revolution */

    double az;
    double el;
    double ror;
    double r;

    double a;
    double b;
    double n0;
    double K2;
    double qd;
    double wd;
    double dc;
    double ci;
    double si;
    double ci2;
    
    LINK link[8];
} SATELLITE;
