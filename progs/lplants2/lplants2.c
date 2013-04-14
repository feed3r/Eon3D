/*
 *  lplants2. Based from
 *  LPLANTS  (see README.lplants)
 *
 *  (C) jschulen\{at}gmx\{dot}de for lplants
 *  (C) 2013 fromani\{at}gmail\{dot}com for lplants2
 *    (cleanup, modernization, Eon3D port and much more)
 *
 * lplants/lplants2 is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * lplants/lplants2 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
/*
    L-string is interpreted by a 3D-turtle

   local  = turtles coordinates: H=aHead, L=Left, U=Up (turtle->hlu)
   global = viewers coordinates: X=right, Y=up, Z=toward(depht)
      gravity points to -Y = (0,-1,0)
    
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define version_string "0.2.6"
#define E0(string) fprintf(stderr,"ERROR: " string "\n")
#define F0(string) { E0(string); exit(1); }
#define RAND( max ) ((max)?rand()%max:0)

FILE *fo;                  /* output-file */
char *oname="lplants.out"; /* output-file-name */
int depth=3, o_mode=0, o_fmt=0, /* length of steps, options, output format */
    sx=0, sy=0, sz=0,         /* startpos */
    clx=0, cly=400, clz=-400, /* camera location */
    cax=0, cay=300, caz=0;    /* camera look_at */

enum {
    NumA = 4,
    STACKSIZE = 256,
    NUMCOL = 16,
    NUMRULES = 256,
    MAXPOLY = 128
};

struct tturtle {         /* turtle status */
  /* double? float would be enough! */
  double x,y,z;          /* turte-pos */
  double hlu[3][3];      /* local coordinate system, direction head,left,up */
  int    col;            /* color = 0..(NUMCOL-1) */
  double ld;             /* line thickness */
  double a[NumA];        /* variable (could be used as age, geta, seta) */
  double angle, llen;	 /* unit of rotation and length */
};

struct tturtle stk[STACKSIZE], /* stack for storing turtle states */
              *turtle;         /* aktual turtle state points to stk[sp] */
int sp;                        /* stack pointer */

/*
  if something like (23.45) is found in string, it is stored to the
  parameter stack and used by the next command
*/  
double p_stack[STACKSIZE];
int    p_sp=0;

int rgb[NUMCOL][6]; /* RGB-palette (0..255) idx3..5 is for random offset */
int minx=999, maxx=-999,
    miny=999, maxy=-999,
    minz=999, maxz=-999;
int sp,slen;   /* stack-pointer and string len */

char  gx[NUMRULES];  /* replacable chars (alphabet) */
char *gs[NUMRULES];  /* production rules */
char gsp[NUMRULES];  /* probabilities in 0.01 (0..100) */

/*
    current polygon in 3D (0..ppoly->len)
    
     vertex 0..2, normal 3..5 (used be smooth triangle)
 */
struct poly {
  struct poly *next;
  struct poly *prev;
  int len;
  double vertex[MAXPOLY][6];
};

struct poly *ppoly=NULL;

/* create a polygon, put last to stack */
struct poly *create_poly( struct poly *last ){
  struct poly *tpoly;
  if ( last && last->next ) return last->next; /* take leaved if available */
  tpoly=(struct poly *)malloc(sizeof(struct poly));
  if(!tpoly){ E0("could not allocate memory for new polygon");return 0; }
  tpoly->next=NULL;
  tpoly->prev=last;
  tpoly->len=0;
  if ( last ) last->next=tpoly;
  return tpoly;
}
/* leave the actual polygon, return to stacked */
struct poly *leave_poly( struct poly *last ){
  if ( last && last->prev ) return last->prev;
  return last; /* do not leave the root! */
}

/*
   in: stk[],llen, poly3[],ppoly->len, pg,
   outputrange 0..500, 0..800
*/
int out_ps(int f){ /* output polyeder in PS-file f=figure: 0=init,1=end */
  int i,j; double x,y,z,cr,cg,cb;
  j=turtle->col; /* add noise */
  cr=(rgb[j][0]+RAND(rgb[j][3]))/255.; if(cr>1)cr=1; if(cr<0)cr=0;
  cg=(rgb[j][1]+RAND(rgb[j][4]))/255.; if(cg>1)cg=1; if(cg<0)cg=0;
  cb=(rgb[j][2]+RAND(rgb[j][5]))/255.; if(cb>1)cb=1; if(cb<0)cb=0;
  switch (f) {
  case 0:{  /* --- open PS-file --- */
     fprintf(fo,
     "%%!PS-Adobe-2.0 EPSF-2.0\n"
     "%%%%Title: %s\n"
     "%%%%Creator:  LPLANTS " version_string "\n"
     "%%%%CreationDate: " __DATE__ "\n"
     "%%%%Pages: 1\n"
     "%%%%BoundingBox: %d %d %d %d\n"
     "%%%%EndComments\n",
      oname,
      ((minx<999)?minx+40:  0), /* influence of projection is estimated */
      ((minx<999)?miny-10:  0),
      ((minx<999)?maxx+60:595),
      ((minx<999)?maxy+60:842) );
     fprintf(fo,
      "%%%%BeginProcSet\n"
      "/dreh %% --- 3D-rotation ---\n"
      "%% x y z wz wx -> x*cz+y*sz (y*cz-x*sz)*cx+z*sx z*cx-(y*cz-x*sz)*sx\n"
      "{ dup sin exch cos 6 2 roll dup sin exch cos dup 5 index mul\n"
      "  4 index 3 index mul add exch 5 4 roll mul 3 2 roll     5 4 roll\n"
      "  mul sub dup 5 index mul exch 4 index  mul 6 3 roll dup 4 3 roll\n"
      "  mul 3 1 roll mul 4 3 roll sub 3 1 roll add exch } def\n"
      "/z %% --- central-projection (0,-800,0)=camera position ---\n");
     fprintf(fo,  /* --- max. string length for ISO-C89 is 509 --- */
      "%% x y z -> x*ay/(y+ay) z*ay/(y+ay) %% mit y<->z\n"
      "{ exch 0 0 dreh 800 %% campos\n"
      "  dup dup 5 4 roll add dup 5 3 roll mul exch div\n"
      "          4 2 roll mul 3 2 roll div exch } def\n"
      "%% x1 y1 z1 .. xn yn zn n -> fillpoly[n]\n");
     fprintf(fo,  /* --- max. string length for ISO-C89 is 509 --- */
      "/p { newpath 1 exch 1 exch\n"
      "    { dup 1 eq { pop z moveto 1 } if\n"
      "          1 ne {     z lineto   } if\n"
      "    } for closepath fill } def\n");
    fprintf(fo,
      "%% x1 y1 z1 ... xn yn zn n -> poly[n]\n"
      "/s { newpath 1 exch 1 exch\n"
      "    { dup 1 eq { pop z moveto 1 } if\n"
      "          1 ne {     z lineto   } if\n"
      "    } for closepath stroke } def\n"
      "/f { setrgbcolor } def\n"
      "/w { setlinewidth } def\n"
      "%%%%EndProcSet\n"
      "50 50 translate\n");
    } break;
  case 1:{                               /* --- close PS-file --- */
     fprintf(fo,"showpage\n");
    } break;
  case 2:{                               /* --- line --- */
     x=turtle->hlu[0][0]*turtle->llen+turtle->x;
     y=turtle->hlu[0][1]*turtle->llen+turtle->y;
     z=turtle->hlu[0][2]*turtle->llen+turtle->z;
     fprintf(fo,"%4.2f %4.2f %4.2f f %4.1f w"
               " %11.6f %11.6f %11.6f"
               " %11.6f %11.6f %11.6f %2d s 1 w\n",
       cr, cg, cb, turtle->ld, /* color + thickness */
       turtle->x, turtle->y, turtle->z, x, y, z, 2); /* r1 r2 numpoints=2 */
    } break;
  case 3:{                               /* --- polyfill --- */
     fprintf(fo,"%4.2f %4.2f %4.2f f",cr,cg,cb); /* color */
     for (i=0;i<ppoly->len;i++)
     fprintf(fo," %11.6f %11.6f %11.6f", /* x y z */
       ppoly->vertex[i][0],
       ppoly->vertex[i][1],
       ppoly->vertex[i][2]);
     fprintf(fo," %3d p\n", ppoly->len); /* n */
    } break;
  }
  return 0;
 }
 
int out_pov(int f){ /* output polyeder f=figure: 0=init,1=end */
  int i; double x,y,z;
  switch (f) {
  case 0:{
     fprintf(fo,
      "// LPLANTS " version_string " output for POVRAY 3.0\n\n"
      "#include \"colors.inc\"\n\n"
      "camera {\n"
      " location <%d+400*sin(radians(360*clock)),%d,%d*cos(radians(360*clock))>\n"
      " look_at  <%d,%d,%d>\n"
      " angle 90\n"
      "}\n", clx, cly, clz,  cax, cay, caz);
     fprintf(fo, /* --- max. ISO-C89 string length is 509 --- */
      "background { color rgb<.25,.25,.5> }\n"
      "// remove crand on movies (flicker effect)\n"
      "plane { <0,1,0>, 0 \n"
      "  pigment { color rgb<0.75,0.45,0.0625> }\n"
      "  finish { crand 0.05 }\n"
      "}\n"
      "// declare random stream used by rand(R1)\n"
      "// unfortunately rand is only called during declare, useless!\n"
      "#declare R1 = seed(0);\n"
      "light_source { <100,800,-300> color rgb<1,1,1> }\n");
     for( i=0; i<NUMCOL; i++ ){
       fprintf(fo,  /* rand is called only in the declaration => to change */
         "#declare text%d= texture {"
         " pigment { color rgb<%.2g+%.2g*rand(R1),"
                              "%.2g+%.2g*rand(R1),"
                              "%.2g+%.2g*rand(R1)> }"
         " finish { ambient 0.5 diffuse 0.5 }"
         "}\n",
         i, rgb[i][0]/255.,rgb[i][3]/255.,
            rgb[i][1]/255.,rgb[i][4]/255.,
            rgb[i][2]/255.,rgb[i][5]/255.
       );     
     }
    } break;
  case 1:{                               /* --- close POV-file --- */
     fprintf(fo,"\n");
    } break;
  /*
     ambient=1, diffuse=0 => all pixel same color
  */
  case 2: if ( turtle->llen>1e-3 ) {      /* --- poly as cone --- */
     x=turtle->hlu[0][0]*turtle->llen+turtle->x;
     y=turtle->hlu[0][1]*turtle->llen+turtle->y;
     z=turtle->hlu[0][2]*turtle->llen+turtle->z;
     fprintf(fo,"cone {\n <%.6g, %.6g, %.6g>, %.4g\n"
                        " <%.6g, %.6g, %.6g>, %.4g\n"
                " texture { text%d }\n}\n",
      turtle->x, turtle->y, turtle->z, turtle->ld,
      x,         y,         z,         turtle->ld, turtle->col);
    } break;
  case 3: if (ppoly->len>2) {             /* --- polyfill as mesh --- */
     fprintf(fo,"mesh {\n");
     for (i=0;i<ppoly->len-2;i++)
     fprintf(fo," smooth_triangle {"
       " <%.6g, %.6g, %.6g>, <%.6g, %.6g, %.6g>, <%.6g, %.6g, %.6g>,"
       " <%.6g, %.6g, %.6g>, <%.6g, %.6g, %.6g>, <%.6g, %.6g, %.6g> }\n",
       ppoly->vertex[  0][0],ppoly->vertex[  0][1],ppoly->vertex[  0][2],
       ppoly->vertex[  0][3],ppoly->vertex[  0][4],ppoly->vertex[  0][5],
       ppoly->vertex[i+1][0],ppoly->vertex[i+1][1],ppoly->vertex[i+1][2],
       ppoly->vertex[i+1][3],ppoly->vertex[i+1][4],ppoly->vertex[i+1][5],
       ppoly->vertex[i+2][0],ppoly->vertex[i+2][1],ppoly->vertex[i+2][2],
       ppoly->vertex[i+2][3],ppoly->vertex[i+2][4],ppoly->vertex[i+2][5]);
     fprintf(fo," texture { text%d }\n}\n",turtle->col);
    } break;
  }
  return 0;
 }


/* output polyeder data for awk f=figure: 0=init,1=end */
/* ToDo: output xyz-min/max-BoundingBox as comment */
int out_data(int f){
  int i; double x,y,z;
  switch (f) {
  case 0:
     fprintf(fo,"# lplants raw data\n");
     fprintf(fo,"# cone:     2  x1 y1 z1  x2 y2 z2  color\n");
     fprintf(fo,"# polyfill: n  x1 y1 z1  ...  xn yn zn  color\n");
     break;
  case 1: break;
  case 2:{                               /* --- poly --- */
     x=turtle->hlu[0][0]*turtle->llen+turtle->x;
     y=turtle->hlu[0][1]*turtle->llen+turtle->y;
     z=turtle->hlu[0][2]*turtle->llen+turtle->z;
     fprintf(fo," 2  %13.8f %13.8f %13.8f"
                  "  %13.8f %13.8f %13.8f"
                  "  %2d\n",
       turtle->x,turtle->y,turtle->z,x,y,z,turtle->col);
    } break;
  case 3:{                               /* --- polyfill --- */
     fprintf(fo,"%2d",ppoly->len);
     for (i=0;i<ppoly->len;i++)
     fprintf(fo,"  %13.8f %13.8f %13.8f",
       ppoly->vertex[i][0],
       ppoly->vertex[i][1],
       ppoly->vertex[i][2]);
     fprintf(fo,"  %2d\n",turtle->col);
    } break;
  }
  return 0;
 }

/* 
   i: 0=header, 1=foot, 2=line/cone, 3=polygon
*/
int draw(int i){
  if( o_fmt==1 ) return out_ps(i);
  if( o_fmt==2 ) return out_pov(i);
  if( o_fmt==3 ) return out_data(i);
  return 0; /* no output, used to calculate box! */
}

/*
    store_vertex to prepare a more complex polygon
    fkt: 0=new_polygon=reset,
         1=update_last_vertex,
         2=set_next_vertex
 */ 
int store_vertex( int fkt ){
  int i;
  switch ( fkt ) {
   case 0: i=0; ppoly->len=1; break;
   case 1: i=ppoly->len-1; if(i<0) i=0; break;
   case 2: i=ppoly->len; ppoly->len++;
           if (ppoly->len>=MAXPOLY) F0("polygon overflow");
           break;
   default: F0("fkt out of range");
  }
 
  if( fkt!=1 ){   /* do not change position after rotations {[f]+f} */
    ppoly->vertex[i][0]=turtle->x;
    ppoly->vertex[i][1]=turtle->y;
    ppoly->vertex[i][2]=turtle->z;
  }
  ppoly->vertex[i][3]=turtle->hlu[2][0];
  ppoly->vertex[i][4]=turtle->hlu[2][1];
  ppoly->vertex[i][5]=turtle->hlu[2][2];

  return 0;
}

/* F: go forward (llen,0,0)_(turtle-xyz);
   mode: 1=store polygon vertex
 */
int  forward( int mode ){
  double x,y,z,buf;
  /* parameter stack p_sp is set by (number) and consumed here */
  buf=turtle->llen; if ( p_sp>0 ) { turtle->llen*=p_stack[--p_sp]; }
  if ( turtle->llen>0.01 ){

   /* lokal orientation (head=hlu[0])*step + lokal position = new position */
   turtle->x = x = turtle->hlu[0][0] * turtle->llen  + turtle->x;
   turtle->y = y = turtle->hlu[0][1] * turtle->llen  + turtle->y;
   turtle->z = z = turtle->hlu[0][2] * turtle->llen  + turtle->z;

   if (mode>0 && ppoly->len>0) { store_vertex( 2 ); } /* add next poly-point */

   if ( x>maxx ) maxx=(int)x;  if ( x<minx ) minx=(int)x;
   if ( y>maxy ) maxy=(int)y;  if ( y<miny ) miny=(int)y;
   if ( z>maxz ) maxz=(int)z;  if ( z<minz ) minz=(int)z;
  }
  turtle->llen=buf;
  return 0;
}

/* rotate turtle around axis(i1,i2): x(1,2), y(2,0), z(0,1)=right/left */
/* axis 0: i1=1 i2=2 around head (rolling right/left)
 *      1: i1=2 i2=0 around left (up/down)
 *      2: i1=0 i2=1 around up   (right/left)
 */
int rotate( double w, int axis){
  int i, i1, i2;
  double b1,b2,cw,sw,ww,buf;
  buf=w; if ( p_sp>0 ) { w=p_stack[--p_sp]*M_PI/180; }
  i1=(axis+1)%3; /* first row of 2x2 rotation matrix */
  i2=(axis+2)%3; /* 2nd   row of 2x2 rotation matrix */
  ww = w ;
  cw = cos(ww);
  sw = sin(ww);
  for ( i=0; i<3; i++ ) {
   b1 = turtle->hlu[i1][i];
   b2 = turtle->hlu[i2][i];
   turtle->hlu[i1][i] = cw*b1 - sw*b2;
   turtle->hlu[i2][i] = sw*b1 + cw*b2;
  }
  w=buf;
  if (ppoly->len>0) { store_vertex( 1 ); } /* update last poly-point */
  return 0;
 }

/* rotate turtle to y-direction (direction+=y) around (Z,0,X)
  
   Rotation around t in direction u=(x,y,z):
   
    x*x+(1-x*x)*cw, x*y*(1-cw)-z*sw, z*x*(1-cw)+y*sw
    x*y*(1-cw)+z*sw, y*y+(1-y*y)*cw, y*z*(1-cw)-x*sw
    z*x*(1-cw)-y*sw, y*z*(1-cw)+x*sw, z*z+cw*(1-z*z)

    x*x*(1-cw)+1*cw, x*y*(1-cw)-z*sw, z*x*(1-cw)+y*sw
    x*y*(1-cw)+z*sw, y*y*(1-cw)+1*cw, y*z*(1-cw)-x*sw
    z*x*(1-cw)-y*sw, y*z*(1-cw)+x*sw, z*z*(1-cw)+1*cw

    Z*Z*(1-cw)+1*cw,-X*sw, X*Z*(1-cw)
               X*sw, 1*cw,           -Z*sw
    X*Z*(1-cw)     , Z*sw, X*X*(1-cw)+1*cw

 */
int gravity( double w ){
  int i;
  double b1,b2,b3,ww,buf,cw,sw,x,y,z,r;
  buf=w; if ( p_sp>0 ) { w=p_stack[--p_sp]*M_PI/180; }
  ww = w; 
  cw = cos(ww);
  sw = sin(ww);
  x = turtle->hlu[0][0];
  y = 0;                  /* vertical (?) component set to 0 */
  z = turtle->hlu[0][2];
  r = x*x + y*y + z*z;
#ifdef DEBUG
  fprintf(stderr," rot: r=%f y=%f\n",r,turtle->hlu[0][1]);
#endif
  /* only rotate if not already in y-direction ??? */
  if ( r>0.0001 ){
   r=1/sqrt(r);
   x*=r;  /* normalize */
   y*=r;
   z*=r;
   for ( i=0; i<3; i++ ) {
    b1 = turtle->hlu[i][0];
    b2 = turtle->hlu[i][1];
    b3 = turtle->hlu[i][2];
    turtle->hlu[i][0] = (z*z*(1-cw)+cw)*b1 - x*sw*b2 +      x*z*(1-cw)*b3;
    turtle->hlu[i][1] =            x*sw*b1 +   cw*b2 -            z*sw*b3;
    turtle->hlu[i][2] =      x*z*(1-cw)*b1 + z*cw*b2 + (x*x*(1-cw)+cw)*b3;
/*
    r =turtle->hlu[i][0]*turtle->hlu[i][0];
    r+=turtle->hlu[i][1]*turtle->hlu[i][0];
    r+=turtle->hlu[i][2]*turtle->hlu[i][0]; r=1/sqrt(r);
    turtle->hlu[i][0]*=r;
    turtle->hlu[i][1]*=r;
    turtle->hlu[i][2]*=r;
    fprintf(stderr," rot: r=%f y=%f\n",r,turtle->hlu[0][1]);
*/
   }
  }
  w=buf;
  if (ppoly->len>0) { store_vertex( 1 ); } /* update last poly-point */
  return 0;
 }

/*
 * roll L to XZ plane, roll means rotation around H
 * local  = turtles coordinates: H=aHead, L=Left, U=Up (turtle->hlu)
 * global = viewers coordinates: X=right, Y=up, Z=toward(depht)
 *    gravity points to -Y = (0,-1,0)
 *
 *  tABoP p57 (H=const, V=-gravity=Y)
 *   L = (Y x H)/|(Y x H)|     
 *   U = H x L
 *
 *  2009-05-28 this function added
 * ToDo: - $ may be replaced by a set of local and global rotations
 *         for the tree1.in replace + - / by ^ & and rotation around Y
 */
int rollLtoXZ(){
  /* (a0,a1,a2) x (b0,b1,b2) = (a1*b2-a2*b1, a2*b0-a0*b2, a0*b1-a1*b0) */
  /* ( 0, 1, 0) x (H0,H1,H2) = (H2, 0, -H0) */
  /* L = (H2, 0, -H0)/sqrt(H2*H2+H0*H0) */
  /* U = H x L = (H0,H1,H2) x (L0, 0,L2) 
   *           = (H1*L2,H2*L0-H0*L2,H1*L0)
   *           = (-H1*H0, H2*H2+H0*H0, H1*H2)/sqrt(H2*H2+H0*H0)
   */
  double h0,h1,h2,lenL,nfac;
  h0=turtle->hlu[0][0];
  h1=turtle->hlu[0][1];
  h2=turtle->hlu[0][2];
  lenL=sqrt(h0*h0+h2*h2); /* lenght of XZ projection */
  if (lenL<1e-7) return 1; /* do not rotate if H points near to Y */
  nfac=1.0/lenL;
  turtle->hlu[1][0]=  h2*nfac;
  turtle->hlu[1][1]=   0;
  turtle->hlu[1][2]= -h0*nfac;
  turtle->hlu[2][0]= -h1*h0*nfac;
  turtle->hlu[2][1]= (h2*h2+h0*h0)*nfac;
  turtle->hlu[2][2]=  h1*h2*nfac;
  /* ToDo: next line needed? */
  if (ppoly->len>0) { store_vertex( 1 ); } /* update last poly-point */
  return 0;
 }


 /* interprete characters by drawing turtle */
 int zeichne(char cc){
   switch (cc) {
    case 'L':
    case 'R':
    case 'F': draw(2);   /* draw a line */
              forward(1); break;
    case 'l':
    case 'r':
    case 'f': forward(1); break; /* forward + set vertex */
    case 'G': forward(0); break; /* forward but no vertex */
    case '.': if (ppoly->len>0) store_vertex(2); break; /* add vertex */
    case '$': rollLtoXZ(); break; /* roll around H to get L in XZ plane */
    case 'g': gravity(-turtle->angle); break; /* down to earth */
    case '+': rotate(-turtle->angle,2); break; /* turn left, around U */
    case '-': rotate(+turtle->angle,2); break; /* turn right */
    case '&': rotate(-turtle->angle,1); break; /* pitch down, around L */
    case '^': rotate(+turtle->angle,1); break; /* pitch up */
    case '\\':rotate(+turtle->angle,0); break; /* roll left, around H */
    case '/': rotate(-turtle->angle,0); break; /* roll right */
    case '|': rotate(          M_PI,2); break; /* turn around */
    case ']': if(sp){ sp--; turtle--; } break; /* pop  state */
    case '[':                                  /* push state */
      sp++; if(sp>=STACKSIZE) F0("TURTLE-STK-OVL");
      memcpy(&stk[sp],turtle,sizeof(struct tturtle)); turtle++; 
      break;
    case '{':
       ppoly=create_poly(ppoly);
       store_vertex( 0 ); break;         /* set 1st point of new polygon */
    case '}': draw(3); ppoly->len=0;
       ppoly=leave_poly(ppoly); break;   /* reset, polygon=off */
    case '"':
      if ( turtle->col<NUMCOL ) turtle->col++;  /* increment color index */
      if ( p_sp>0 ) { turtle->col = (int)(p_stack[--p_sp]) % NUMCOL; }
      break;
    case '!':  /* reduce line thickness */
      /* if ( turtle->ld>0 )       turtle->ld-=1.0; pre2009-05-28 */
      if ( turtle->ld>0 )       turtle->ld*=0.707; /* since 2009-05-28 */
      if ( p_sp>0 ) { turtle->ld = p_stack[--p_sp]; }
      break;
    default: break;
   }
   return 0;
}

/*
 *  parameter stack operations
 */

double p_get(){
  if ( p_sp==0 ) { F0("p_stack underrun"); } /* code-optimation: rare */
  return p_stack[--p_sp];
}

void p_put( double value ){
  if ( p_sp>=STACKSIZE ) { F0("p_stack overrun"); } /* code-optimation: rare */
  p_stack[p_sp++]=value;
  return;
}

/*
 *  interpret an expression (postfix, forth-like)
 *   age = rekurion depth, depth ... 0
 *   
 *   x / y ==> y x /
 *   x - y ==> y x -
 *   
 */
int expression( char *s, int len, int age ) {
  int i,j; double rr;
#ifdef DEBUG_EXPRESSION
  fprintf(stderr," age=%2d expression[%2d]=%s\n",age,len,s);
#endif
  for( i=0; i<len; ){
    for (    ; i<len && s[i]==' '; i++ ); /* ignore spaces */
    for ( j=i; j<len && s[j]!=' '; j++ ); /* measure length */
    
    if ( strstr(s+i,"dup")==s+i ){ rr=p_get();p_put(rr);p_put(rr); } else
    if ( strstr(s+i,"rnd")==s+i ){ p_put( RAND(256)/256. ); } else
    if ( strstr(s+i,"age")==s+i ){ p_put( 1.0*age ); } else
    if ( strstr(s+i,"getl")==s+i ){ p_put(turtle->llen); } else
    if ( strstr(s+i,"setl")==s+i ){ turtle->llen=p_get(); } else
    if ( strstr(s+i,"gett")==s+i ){ p_put(turtle->ld); } else
    if ( strstr(s+i,"sett")==s+i ){ turtle->ld=p_get(); } else
    /* instead of using only register a we could use a00..a99 */
    if ( strstr(s+i,"geta")==s+i ){ p_put(turtle->a[atoi(s+i+4)%NumA]);  } else
    if ( strstr(s+i,"seta")==s+i ){ turtle->a[atoi(s+i+4)%NumA]=p_get(); } else
    if ( strstr(s+i,"print")==s+i ){ rr=p_get(); p_put(rr); fprintf(stderr," %g",rr); } else
    if ( strstr(s+i,"+")==s+i && j-i==1 ){ p_put( p_get()+p_get() ); } else
    if ( strstr(s+i,"-")==s+i && j-i==1 ){ p_put( p_get()-p_get() ); } else
    if ( strstr(s+i,"*")==s+i && j-i==1 ){ p_put( p_get()*p_get() ); } else
    if ( strstr(s+i,"/")==s+i && j-i==1 ){ p_put( p_get()/p_get() ); } else
    if ( strstr(s+i,"pow")==s+i ){ p_put( pow(p_get(),p_get()) ); } else
    /* else: double value */ { p_put( atof(s+i) ); }

    i=j;
  }
  return 0;
}

/* rekursive string interpreter; id=depth; s=string */
int interpret( int id, char *s){
  int i,llen,rule; char *s2;
  llen=strlen(s);
  for ( i=0; i<llen; i++ ){
    if (s[i]==' ') continue; /* ignore spaces */
    if (s[i]=='('){   /* parameter (xxx) found, no nesting possible */
     s2=strchr(s+i+1,')'); if (!s2) F0("missing )");

     /* interpreting expression between brackets (subfunction?) */
     /* if ( id==0 ) */ {
       *s2=0;
       if ( o_mode&1 ) printf("(%s)",s+i+1); /* for debugging */
       expression(s+i+1,s2-s-i-1,id); /* put result to the stack */
       *s2=')';
     }

     i=s2-s; /* for(;;i++) is executed! */
     continue;
    }
    s2=strchr(gx,s[i]);       /* is char s[i] replaceable? */
    if ( id==0 || s2==0 ) {   /* if not, try to interpret it */
      if ( o_mode&1 ) printf("%c",s[i]); /* for debugging */
      zeichne(s[i]);
    } else {
      rule=s2-gx;
      if( gsp[rule]>=100 || RAND(100)<gsp[rule] ) interpret(id-1,gs[rule]);
    }
  }
  slen+=llen;
  return 0;
 }


/* load base rules, DNA, DOL */
int read_dol(FILE *fi){
  int i,j,i1,gslen;
  double angle, llen, ld;
  char s1[256], *s2;
  { 
   gx[0]=0; gslen=0;
   for ( i=0; i<NUMCOL;         i++ ) rgb[i][0]=rgb[i][1]=rgb[i][2]=i;
   for ( i=0; i<NUMCOL;         i++ ) rgb[i][3]=rgb[i][4]=rgb[i][5]=0;
   for ( i=0; i<NUMCOL && i<16; i++ ) rgb[i][0]=rgb[i][1]=rgb[i][2]=16*i;
   sx=sy=sz=0; llen=7; ld=0; angle=15*M_PI/180;
   while ( ! feof(fi) ) {
    if( ! fgets(s1,255,fi) ) break;
#ifdef DEBUG
    printf("# read: %s",s1);
#endif
    s2=strstr(s1,";");  if (s2) *s2=0;
    s2=strstr(s1,"\n"); if (s2) *s2=0;
    s2=strstr(s1,"startpos="); if(s2){ sscanf(s2+9,"%d%d%d",&sx,&sy,&sz); }
    s2=strstr(s1,"camera=");   if(s2){ sscanf(s2+7,"%d%d%d%d%d%d",&clx,&cly,&clz,&cax,&cay,&caz); }
    s2=strstr(s1,"angle=");    if(s2){ sscanf(s2+6,"%lg",&angle); angle*=M_PI/180; }
    s2=strstr(s1,"unitlen=");  if(s2){ sscanf(s2+8,"%lg",&llen); }
    s2=strstr(s1,"depth=");    if(s2){ sscanf(s2+6,"%d",&depth); }
    s2=strstr(s1,"thick=");    if(s2){ sscanf(s2+6,"%lg",&ld); }
    s2=strstr(s1,"color=");    if(s2){ sscanf(s2+6,"%d",&i1);
      if(i1<NUMCOL) 
       sscanf(s2+6,"%*d%d%d%d%d%d%d",&rgb[i1][0],&rgb[i1][1],&rgb[i1][2],
                                     &rgb[i1][3],&rgb[i1][4],&rgb[i1][5]);
    }
    s2=strstr(s1," -> ");      if(s2){ /* load rules */
      gx[gslen]=s1[0]; gsp[gslen]=100; 
      if ( s2-s1>3 ){ gsp[gslen]=atoi(s1+2); }
      if ( gsp[gslen]<0 || gsp[gslen]>100 ) gsp[gslen]=100;
      gs[gslen]=malloc(strlen(s2)-4+1);
      strncpy(gs[gslen],s2+4,strlen(s2)-4);
      gslen++; if( gslen>= NUMRULES ) F0("NUMRULES exceeded");
    }
   }
  }
  if( o_mode&2 ){
    /* output a '#' in front of every verbose line (for filtering) */
    printf("# num_rules: %d\n",gslen);
    printf("# alphabet: %s\n",gx);
    for(i=0;i<gslen;i++)
      printf("# rule %2d: %c -> %s\n",i+1,gx[i],gs[i]);
  }

  /* reset turtle status */
  sp=0;        /* stack pointer */
  turtle=&stk[sp];
  turtle->x=sx; /* -500 ... +500 */
  turtle->y=sy;
  turtle->z=sz;
  for (i=0;i<NumA;i++)
    turtle->a[i]=0.0;
  turtle->col=1;
  turtle->ld=ld;
  turtle->angle=angle;
  turtle->llen=llen;
  for(i=0;i<3;i++) 
  for(j=0;j<3;j++)
    turtle->hlu[i][j]=((i==j)?1:0); /* reset orientation */
  return 0;
}

int sort1(){ return 0; }
int sort3(){ return 0; }

int help(int ret){
  fprintf(stderr,
   "Lindenmayer generator LPLANTS "version_string" (w) Joerg Schulenburg\n"
   " parameters: [options] input-file output-file\n options:\n"
   "  -d<n> - set depth to <n>\n"
   "  -help - this help (-?,-h,-H too)\n"
   "  -L    - output Lindenmayer string\n"
   "  -r<n> - set srand(<n>)\n"
   "  -a<f> - set parameter a0 <f> (float)\n"
   "  -u<n> - set unitlength to <n>\n"
   "  -V    - print version\n"
   "  -v    - be verbose\n"
   "  -q    - be quiet (suppress messages)\n");
  fprintf(stderr,
   " examples:\n"
   "  lplants examples/demo14.in demo14.ps\n"
   "  lplants examples/demo15.in demo15.pov\n"
   "  lplants examples/flower1.in /dev/stdout # raw data output\n"
  );
  if (ret) exit(ret);
  return 0;
}

int main(int argc, char *argv[]){
 int ii=1, o_depth=-1;
 double o_llen=-1, o_pa=0.0;
 char *s1;
 FILE *fi;
 
 if (argc <= 1) help(1);
 while (ii<argc && argv[ii][0]=='-' ) {
   s1=argv[ii];
   if( !strchr("dLmrtuvVq",s1[1]) ) { help(0); return 0; }
   if( s1[1]=='L' ) { o_mode|=1; }
   if( s1[1]=='v' ) { o_mode|=2; }
   if( s1[1]=='d' ) { if(s1[2]!=0) { o_depth=atoi(s1+2); } }
   if( s1[1]=='r' ) { if(s1[2]!=0) { srand(atoi(s1+2)); } }
   if( s1[1]=='a' ) { if(s1[2]!=0) { o_pa=atof(s1+2); } }
   if( s1[1]=='u' ) { if(s1[2]!=0) { o_llen=atof(s1+2); } }
   if( s1[1]=='q' ) { o_mode|=4;o_mode&=~2; }
   if( s1[1]=='V' ) { printf("LPLANTS " version_string "\n"); return 0; }
   ii++;
 } 
 if (argc-ii!=2) help(1);
 
 ppoly=create_poly(ppoly);
 fi=fopen(argv[ii],"rt"); /* --- open input-file --- */
 if(!fi){ F0("could not open input-file"); } else ii++;

 /* read dol-system */
 if( o_mode&2 ) printf("# read DOL\n");
 read_dol(fi);
 if ( fi ) fclose(fi);

 oname=argv[ii];
 if (strstr(oname,".ps" )) { o_fmt=1;     } else
 if (strstr(oname,".eps")) { o_fmt=1+128; } else
 if (strstr(oname,".pov")) { o_fmt=2;     } else
 if (strstr(oname,".PS" )) { o_fmt=1;     } else
 if (strstr(oname,".EPS")) { o_fmt=1+128; } else
 if (strstr(oname,".POV")) { o_fmt=2;     } else o_fmt=3; /* raw */

 fo=fopen(oname,"wt"); /* --- open output-file --- */
 if (!fo) { F0("could not open output-file"); }

 slen=0;    /* reset symbol counter */

 if (o_depth>-1) depth=o_depth;
 if (o_llen >0 ) turtle->llen=o_llen;
 if (o_pa   >0 ) turtle->a[0]=o_pa;

 /* start iteration */
 if( o_mode&2 )
   printf("# start iteration: depth=%d ulen=%g angle=%.1f pa0=%g ld=%g\n",
   depth,turtle->llen,(.5+turtle->angle/M_PI*180),turtle->a[0],turtle->ld);

 if (o_fmt&128)             /* 1st pass to create a box */
 interpret(depth+1,"[S]");  /* L-system generation */

 slen=0;    /* reset symbol counter */

 o_fmt&=127;                /* main pass */
 draw(0);                   /* output header */
 interpret(depth+1,"S");    /* L-system generation */
 draw(1);                   /* output tail */

 if ( fo ) fclose(fo);

 if( o_mode&1 ) printf("\n");
 if( o_mode&2 ) printf("# symbols=%5d box=(%d..%d,%d..%d,%d..%d) p_sp=%d?=0\n",
     slen, minx,maxx, miny,maxy, minz,maxz, p_sp);
 return 0;
}
