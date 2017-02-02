#include <stdio.h>
#include <math.h>
#include <time.h>
#include "sat.h"

double PI2;
extern double sx, sy, sz, az, el, m256, r, vv, alt, vor, ror, fpa;
extern double slat, slon;
extern int rn;
double xx, zz;
double ux, uy, uz, ex, ey, ez, nx, ny, nz;
double ox, oy, oz, vox, voy, voz;

double atan2(y, x)
double x, y;
{
    if (x>0) {
        return atan(y/x);
    }
    if (x<0) {
        if (y>=0) return atan(y/x)+PI;
        if (y<0) return atan(y/x)-PI;
    }
    if (x==0) {
        if (y>0) return PI/2;
        if (y<0) return -PI/2;
        if (y==0) return 0;
    }
}

setObserver(lon, lat, hgt)
double lon;
double lat;
int hgt;
{
    double la, lo;
    double rp, d, rx, rz;
    double W0;

    PI2 = 2.0*PI;
    W0 = WE/86400;
    
    la = rad(lat);
    lo = rad(lon);

    ux = cos(la)*cos(lo);       /* ux */
    uy = cos(la)*sin(lo);       /* uy */
    uz = sin(la);               /* uz */

    ex = -sin(lo);              /* ex */
    ey =  cos(lo);              /* ey */
    ez =  0.0;                  /* ez */

    nx = -sin(la)*cos(lo);      /* nx */
    ny = -sin(la)*sin(lo);      /* ny */
    nz =  cos(la);              /* nz */

    rp = RE*(1-FL);
    xx = RE*RE;
    zz = rp*rp;
    d = sqrt(xx*cos(la)*cos(la) + zz*sin(la)*sin(la));
    rx = xx/d+hgt;
    rz = zz/d+hgt;

    ox = rx*ux;
    oy = rx*uy;
    oz = rz*uz;

    vox = -oy*W0;
    voy =  ox*W0;
    voz =  0;
}

double getDouble(s, i0, i1)
char *s;
int i0, i1;
{
    char buf[20];
    int i;
    
    for (i=0; i+i0<i1; i++) {
        buf[i] = s[i0+i];
    }
    buf[i] = 0;
    return atof(buf);
}

long int getLong(s, i0, i1)
char *s;
int i0, i1;
{
    char buf[20];
    int i;
    
    for (i=0; i+i0<i1; i++) {
        buf[i] = s[i0+i];
    }
    buf[i] = 0;
    return atol(buf);
}

SATELLITE *readTLE(fp)
FILE *fp;
{
    SATELLITE *sat;
    int i, j, len;
    char name[80], line1[80], line2[80];
    double ec, e2, ci, mm, pc, a, b, m2;

    fgets(name,75,fp);
    fgets(line1,75,fp);
    fgets(line2,75,fp);

    if (strlen(line2) < 10)
        return FALSE;
                            
    len=strlen(name);
    while (name[len]==32 || name[len]==0 || name[len]==10 || name[len]==13 || len==0) {
        name[len]=0;
        len--;
    }
                
    sat = (SATELLITE *)malloc(sizeof(SATELLITE));
    memset(sat, 0, sizeof(SATELLITE));
    
    strncpy(sat->name, name, 24);
    sat->ey = getLong(line1, 18, 20);
    sat->ed = getDouble(line1, 20, 32);
    sat->in = getDouble(line2, 8, 16);
    sat->ra = getDouble(line2, 17, 25);
    sat->ec = 1.0e-07*getDouble(line2, 26, 33);
    sat->wp = getDouble(line2, 34, 42);
    sat->ma = getDouble(line2, 43, 51);
    sat->mm = getDouble(line2, 52, 63);
    m2 = 2*PI*getDouble(line1, 33, 43);
    sat->rv = getDouble(line2, 63, 68);
            
    ec = sat->ec;
    a = pow(G0/(sat->mm*sat->mm),1.0/3.0);
    sat->a = a;
    e2 = 1.0-ec*ec;
    b = sat->a*sqrt(e2);
    sat->b = b;
    mm = sat->mm*2*PI;
    sat->n0 = mm/86400;
            
    sat->K2 = 9.95*(pow(RE/a, 3.5)/(e2*e2));
    sat->si = sin(rad(sat->in));
    ci = sat->ci = cos(rad(sat->in));
    sat->ci2 = ci*ci;

    /* precession constant rad/day */
    pc = RE*a/b/b;
    pc = 1.5*J2*pc*pc*mm;

    /* node precession rate */
    sat->qd = -pc*ci;

    /* perigee precession rate */
    sat->wd = pc*(5*ci*ci-1)/2;
    
    sat->dc = -2*m2/mm/3;
    
    return sat;
}

void satvec(sat, tp)
SATELLITE *sat;
struct tm *tp;
{
    char c;
    double t;
    double SG, CG, si, ci, sq, cq, sw, cw, ra, wp;
    double a, b, ec, rxy, sil, ods, wds;
    double OrbitPos, m;
    double d, ea, se, ce, dnom, M1, M5, G7;
    double x, y, z;
    double vx, vy, vz, vsx, vsy, vsz;
    double n, e, u;
    double rx, ry, rz, rs;
    double cx[3], cy[3], cz[3];
    double lat, lon;
    
    t = tp->tm_yday+1 + tp->tm_hour/24.0 +
        tp->tm_min/1440.0 + tp->tm_sec/86400.0;

    if (sat->ey == tp->tm_year-1) {
        t = t+365.0;
        if (sat->ey%4 == 0) {
            t = t+1.0;  
        }
    }
    
    ra = sat->ra-(t-sat->ed)*sat->K2*sat->ci;
    sq = sin(rad(ra));
    cq = cos(rad(ra));

    wp = sat->wp+(t-sat->ed)*sat->K2*(2.5*(sat->ci2)-0.5);
    sw = sin(rad(wp));
    cw = cos(rad(wp));
    
    /* cx[3], cy[3] and cz[3] transform 
       between orbit coords and celestial coords */

    si = sat->si;
    ci = sat->ci;

    cx[0] =  cw*cq - sw*sq*ci;
    cx[1] = -sw*cq - cw*sq*ci;
    cx[2] =  si*sq;
    cy[0] =  cw*sq + sw*cq*ci;
    cy[1] = -sw*sq + cw*cq*ci;
    cy[2] = -si*cq;
    cz[0] =  sw*si;
    cz[1] =  cw*si;
    cz[2] =  ci;
    
    c = getKey();
    if (c>0) return;
    
    OrbitPos = sat->ma/360.0 + sat->rv + sat->mm*(t-sat->ed);
    rn = (int)OrbitPos;
    m256 = (OrbitPos-rn)*256.0;
    m = (OrbitPos-rn)*2.0*PI; 
    
    ec = sat->ec;
    ea = m+ec*sin(m)+0.5*ec*ec*sin(2.0*m);

    do {
        se = sin(ea);
        ce = cos(ea);
        dnom = 1-ec*ce;
        d = (ea-ec*se-m)/dnom;
        ea = ea-d;
    } while (fabs(d)>1E-5);

    c = getKey();
    if (c>0) return;
    
    a = sat->a;
    b = sat->b;
    
    sx = a*(ce-ec);
    sy = b*se;
    vsx = -a*se/dnom*sat->n0;
    vsy =  b*ce/dnom*sat->n0;

    /* satellite in celestial coords */
    x = sx*cx[0] + sy*cx[1];
    y = sx*cy[0] + sy*cy[1];
    z = sx*cz[0] + sy*cz[1];
    
    vx = vsx*cx[0] + vsy*cx[1];
    vy = vsx*cy[0] + vsy*cy[1];
    vz = vsx*cz[0] + vsy*cz[1];
    
    c = getKey();
    if (c>0) return;
    
    /* and in geocentric coordinates */
    G7 = t*G1+gmst((double)(sat->ey+2000));
    G7 = (G7-(int)G7)*2.0*PI;
    SG = -sin(G7);
    CG =  cos(G7);

    sx = x*CG - y*SG;
    sy = x*SG + y*CG;
    sz = z;
    
    rs = sqrt(sx*sx+sy*sy+sz*sz);
    
    lat = deg(asin(sz/rs));
    lon = deg(atan2(sy,sx));
    if (lon<0) lon+=360;

    wds = sat->wd/86400;
    vsx = vx*CG - vy*SG - wds*sy;
    vsy = vx*SG + vy*CG + wds*sx;
    vsz = vz;
    
    c = getKey();
    if (c>0) return;
    
    /* add ascending node precession to velocity */
    ods = sat->qd/86400;
    vsy += ods*sx;
    vsx -= ods*sy;

    /* compute range vector */
    rx = sx-ox;
    ry = sy-oy;
    rz = sz-oz;

    /* compute range magnitude */
    r = sqrt(rx*rx+ry*ry+rz*rz);

    /* normalize r to unit length */
    rx /= r;
    ry /= r;
    rz /= r;

    u = rx*ux+ry*uy+rz*uz;
    e = rx*ex+ry*ey+rz*ez;
    n = rx*nx+ry*ny+rz*nz;

    c = getKey();
    if (c>0) return;
    
    az = deg(atan2(e,n));
    if (az<0) az+=360;
    el = deg(asin(u));
        
    /* compute relative velocity */
    vv = (vsx-vox)*rx + (vsy-voy)*ry + vsz*rz;

    rxy = sx*sx+sy*sy;
    ror = sqrt(rxy+sz*sz);
    rxy = sqrt(rxy);

    slon = deg(atan2(sy, sx));
    if (slon<0) slon+=360;

    c = getKey();
    if (c>0) return;
    
    sil = sz/ror;
    slat = deg(asin(sil));
    
    sil = sil*sil;
    alt = ror - 1.0/sqrt((1.0-sil)/xx + sil/zz);
    vor = sqrt(vsx*vsx + vsy*vsy + vsz*vsz);
    fpa = deg(asin((sx*vsx + sy*vsy + sz*vsz)/ror/vor));
    
    sat->az = az;
    sat->el = el;
    sat->ror= ror-RE;
    sat->r  = r;
}
