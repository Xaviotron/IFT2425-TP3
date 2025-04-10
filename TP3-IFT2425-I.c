//------------------------------------------------------
// module  : Tp-IFT2425-I.c
// author  : François Frigon - 20297551 - francois.frigon@umontreal.ca
//           Xavier Dontigny - 20215658 - xavier.dontigny@umontreal.ca
// date    : 9 avril 2025
// version : 1.0
// language: C++
// note    :
//------------------------------------------------------
//  

//------------------------------------------------
// FICHIERS INCLUS -------------------------------
//------------------------------------------------
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <new>
#include <time.h>

//-------------------------
//--- Windows -------------
//-------------------------
#include <X11/Xutil.h>

Display   *display;
int	  screen_num;
int 	  depth;
Window	  root;
Visual*	  visual;
GC	  gc;

/************************************************************************/
/* OPEN_DISPLAY()							*/
/************************************************************************/
int open_display()
{
  if ((display=XOpenDisplay(NULL))==NULL)
   { printf("Connection impossible\n");
     return(-1); }

  else
   { screen_num=DefaultScreen(display);
     visual=DefaultVisual(display,screen_num);
     depth=DefaultDepth(display,screen_num);
     root=RootWindow(display,screen_num);
     return 0; }
}

/************************************************************************/
/* FABRIQUE_WINDOW()							*/
/* Cette fonction crée une fenetre X et l'affiche à l'écran.	        */
/************************************************************************/
Window fabrique_window(char *nom_fen,int x,int y,int width,int height,int zoom)
{
  Window                 win;
  XSizeHints      size_hints;
  XWMHints          wm_hints;
  XClassHint     class_hints;
  XTextProperty  windowName, iconName;

  char *name=nom_fen;

  if(zoom<0) { width/=-zoom; height/=-zoom; }
  if(zoom>0) { width*=zoom;  height*=zoom;  }

  win=XCreateSimpleWindow(display,root,x,y,width,height,1,0,255);

  size_hints.flags=PPosition|PSize|PMinSize;
  size_hints.min_width=width;
  size_hints.min_height=height;

  XStringListToTextProperty(&name,1,&windowName);
  XStringListToTextProperty(&name,1,&iconName);
  wm_hints.initial_state=NormalState;
  wm_hints.input=True;
  wm_hints.flags=StateHint|InputHint;
  class_hints.res_name=nom_fen;
  class_hints.res_class=nom_fen;

  XSetWMProperties(display,win,&windowName,&iconName,
                   NULL,0,&size_hints,&wm_hints,&class_hints);

  gc=XCreateGC(display,win,0,NULL);

  XSelectInput(display,win,ExposureMask|KeyPressMask|ButtonPressMask| 
               ButtonReleaseMask|ButtonMotionMask|PointerMotionHintMask| 
               StructureNotifyMask);

  XMapWindow(display,win);
  return(win);
}

/****************************************************************************/
/* CREE_XIMAGE()							    */
/* Crée une XImage à partir d'un tableau de float                           */
/* L'image peut subir un zoom.						    */
/****************************************************************************/
XImage* cree_Ximage(float** mat,int z,int length,int width)
{
  int lgth,wdth,lig,col,zoom_col,zoom_lig;
  float somme;
  unsigned char	 pix;
  unsigned char* dat;
  XImage* imageX;

  /*Zoom positiv*/
  /*------------*/
  if (z>0)
  {
   lgth=length*z;
   wdth=width*z;

   dat=(unsigned char*)malloc(lgth*(wdth*4)*sizeof(unsigned char));
   if (dat==NULL)
      { printf("Impossible d'allouer de la memoire.");
        exit(-1); }

  for(lig=0;lig<lgth;lig=lig+z) for(col=0;col<wdth;col=col+z)
   { 
    pix=(unsigned char)mat[lig/z][col/z];
    for(zoom_lig=0;zoom_lig<z;zoom_lig++) for(zoom_col=0;zoom_col<z;zoom_col++)
      { 
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+0)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+1)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+2)]=pix;
       dat[((lig+zoom_lig)*wdth*4)+((4*(col+zoom_col))+3)]=pix; 
       }
    }
  } /*--------------------------------------------------------*/

  /*Zoom negatifv*/
  /*------------*/
  else
  {
   z=-z;
   lgth=(length/z);
   wdth=(width/z);

   dat=(unsigned char*)malloc(lgth*(wdth*4)*sizeof(unsigned char));
   if (dat==NULL)
      { printf("Impossible d'allouer de la memoire.");
        exit(-1); }

  for(lig=0;lig<(lgth*z);lig=lig+z) for(col=0;col<(wdth*z);col=col+z)
   {  
    somme=0.0;
    for(zoom_lig=0;zoom_lig<z;zoom_lig++) for(zoom_col=0;zoom_col<z;zoom_col++)
     somme+=mat[lig+zoom_lig][col+zoom_col];
           
     somme/=(z*z);    
     dat[((lig/z)*wdth*4)+((4*(col/z))+0)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+1)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+2)]=(unsigned char)somme;
     dat[((lig/z)*wdth*4)+((4*(col/z))+3)]=(unsigned char)somme; 
   }
  } /*--------------------------------------------------------*/

  imageX=XCreateImage(display,visual,depth,ZPixmap,0,(char*)dat,wdth,lgth,16,wdth*4);
  return (imageX);
}

//-------------------------//
//-- Matrice de Flottant --//
//-------------------------//
//---------------------------------------------------------
//  Alloue de la memoire pour une matrice 1d de float
//----------------------------------------------------------
float* fmatrix_allocate_1d(int hsize)
 {
  float* matrix;
  matrix=new float[hsize]; return matrix; }

//----------------------------------------------------------
//  Alloue de la memoire pour une matrice 2d de float
//----------------------------------------------------------
float** fmatrix_allocate_2d(int vsize,int hsize)
 {
  float** matrix;
  float *imptr;

  matrix=new float*[vsize];
  imptr=new  float[(hsize)*(vsize)];
  for(int i=0;i<vsize;i++,imptr+=hsize) matrix[i]=imptr;
  return matrix;
 }

//----------------------------------------------------------
// Libere la memoire de la matrice 1d de float
//----------------------------------------------------------
void free_fmatrix_1d(float* pmat)
{ delete[] pmat; }

//----------------------------------------------------------
// Libere la memoire de la matrice 2d de float
//----------------------------------------------------------
void free_fmatrix_2d(float** pmat)
{ delete[] (pmat[0]);
  delete[] pmat;}

//----------------------------------------------------------
// Sauvegarde de l'image de nom <name> au format pgm                        
//----------------------------------------------------------                
void SaveImagePgm(char* bruit,char* name,float** mat,int lgth,int wdth)
{
 int i,j;
 char buff[300];
 FILE* fic;

  //--extension--
  strcpy(buff,bruit);
  strcat(buff,name);
  strcat(buff,".pgm");

  //--ouverture fichier--
  fic=fopen(buff,"wb");
    if (fic==NULL) 
        { printf("Probleme dans la sauvegarde de %s",buff); 
          exit(-1); }
  printf("\n Sauvegarde de %s au format pgm\n",buff);

  //--sauvegarde de l'entete--
  fprintf(fic,"P5");
  fprintf(fic,"\n# IMG Module");
  fprintf(fic,"\n%d %d",wdth,lgth);
  fprintf(fic,"\n255\n");

  //--enregistrement--
  for(i=0;i<lgth;i++) for(j=0;j<wdth;j++) 
	fprintf(fic,"%c",(char)mat[i][j]);
   
  //--fermeture fichier--
   fclose(fic); 
}

//-------------------------//
//---- Fonction Pour TP ---//
//-------------------------//

 //>Var

// Question 1.1
  //>Cst
  const double PI=3.14159265358979323846264338;
  int NBINTERV=5000000;
  int NbInt=NBINTERV;
 

 float* VctPts=fmatrix_allocate_1d(NbInt+1);

// Calcul de la fonction à intégrer
float f(float x) { return (4.0f * sqrt(1 - x*x)); }

//Calcul de l'intégrale
float calculIntegrale(int NbInt)
{
  float result = 0.0; 
  double result_;
  float a = 0.0;   // Borne inférieure
  float b = 1.0;   // Borne supérieure
  float h = (b-a) / NbInt;  // Largeur de chaque trapèze
  float x;

  for (int i=1; i<NbInt; i++) {
    x = i * h;
    float fx = f(x);
    result += fx;
    VctPts[i] = fx; 

  }

  VctPts[0] = f(a);
  VctPts[NbInt] = f(b);

   result += 0.5 * (f(a) + f(b));

   result *= h;
   
   return result;
  }

  // Question 1.2

  float sommeParPaires(float* VctPts, int start, int end) 
{
    int length = end - start + 1;
    
    if (length <= 2) {
        float somme = 0.0;
        for (int i = start; i <= end; i++) {
            somme += VctPts[i];
        }
        return somme/NBINTERV;
    }
    
    int mid = start + (length / 2) - 1;
    float somme = 0.0;
    
    somme += sommeParPaires(VctPts, start, mid);
    somme += sommeParPaires(VctPts, mid + 1, end);
    
    return somme;
}

float sommeKahan(float* VctPts, int start, int end) 
{
    float e = 0.0; // Compensation pour les erreurs d'arrondi
    float sommeKahan = 0.0;
    
    for (int i = start; i <= end; i++) {
        float y = VctPts[i] - e; // Compensation
        float t = sommeKahan + y; // Somme partielle
        e = (t - sommeKahan) - y; // Nouvelle compensation
        sommeKahan = t; // Mise à jour de la somme
    }
    
    return sommeKahan/NBINTERV; // Normalisation
}

void diagrammeBifurcation(float** Graph2D, int width, int height) {
    float mu_min = 2.5f;
    float mu_max = 4.0f;
    float dmu = (mu_max - mu_min) / (width - 1);

    // Initialisation image blanche
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            Graph2D[i][j] = 255.0f;
        }
    }

    for (int i = 0; i < width; i++) {
        float mu = mu_min + i * dmu;
        float x = 0.5f;  // x0

        // skip les 10000 premières itérations
        for (int n = 0; n < 10000; n++) {
            x = mu * x * (1.0f - x);
        }

        // draw les 10000 itérations suivantes
        for (int n = 0; n < 10000; n++) {
            x = mu * x * (1.0f - x);
            int y_pixel = height - 1 - (int)(x * height);
            if (y_pixel >= 0 && y_pixel < height) {
                Graph2D[y_pixel][i] = 0.0f; // noir
            }
        }
    }
}

void estimePi() {
  const int N = 10000000;
  const int N1 = 10000;         // skip
  float x_float[3] = {0.2f, 0.4f, 0.6f};
  double x_double[3] = {0.2, 0.4, 0.6};

  for (int i = 0; i < 3; i++) {

      float sum_float = 0.0f;
      float x_f = x_float[i];

      // Skip
      for (int n = 0; n < N1; n++) {
          x_f = 4.0f * x_f * (1.0f - x_f);
      }

      for (int n = 0; n < N; n++) {
          x_f = 4.0f * x_f * (1.0f - x_f);
          sum_float += sqrtf(x_f);
      }

      // Approx en float
      float pi_float = 2.0f * (N / sum_float);

      double sum_double = 0.0;
      double x_d = x_double[i];

      // Skip
      for (int n = 0; n < N1; n++) {
          x_d = 4.0 * x_d * (1.0 - x_d);
      }

      for (int n = 0; n < N; n++) {
          x_d = 4.0 * x_d * (1.0 - x_d);
          sum_double += sqrt(x_d);
      }

      // Approx en double
      double pi_double = 2.0 * (N / sum_double);


      printf("x0=%.1f | Float: π≈%.10f, Double: π≈%.10f\n",
             x_float[i], pi_float, pi_double);
  }
}
    


//----------------------------------------------------------
//----------------------------------------------------------
// PROGRAMME PRINCIPAL -------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
int main(int argc,char** argv)
{
 int   i,j,k,l;
 int   flag_graph;
 int   zoom;

 //Pour Xwindow
 //------------
 XEvent ev;
 Window win_ppicture;
 XImage *x_ppicture; 
 char   nomfen_ppicture[100]; 
 int    length,width;

 length=width=4096;
 float** Graph2D=fmatrix_allocate_2d(length,width); 
 flag_graph=1;
 zoom=-16;

 //Affichage Axes
 for(i=0;i<length;i++) for(j=0;j<width;j++) Graph2D[i][j]=190.0;


//--------------------------------------------------------------------------------
// PROGRAMME ---------------------------------------------------------------------
//--------------------------------------------------------------------------------






 //Programmer ici

 if (argc>1)  { NbInt=atoi(argv[1]); }

 // Question 1.1
 float result = calculIntegrale(5000000);
 printf("L'intégrale est : %.10f\n", result);

// Question 1.2
  float somme = sommeParPaires(VctPts, 0, NbInt);
  printf("La somme par paires est : %.10f\n", somme);

  float sommeKahanResult = sommeKahan(VctPts, 0, NbInt);
  printf("La somme Kahan est : %.10f\n", sommeKahanResult);

// 3
diagrammeBifurcation(Graph2D, 4096, 4096);

//4
estimePi();

 //End
   

//--------------------------------------------------------------------------------
//---------------- visu sous XWINDOW ---------------------------------------------
//--------------------------------------------------------------------------------
 if (flag_graph)
 {
 //ouverture session graphique
 if (open_display()<0) printf(" Impossible d'ouvrir une session graphique");
 sprintf(nomfen_ppicture,"Graphe : ","");
 win_ppicture=fabrique_window(nomfen_ppicture,10,10,width,length,zoom);
 x_ppicture=cree_Ximage(Graph2D,zoom,length,width);

 //Sauvegarde
 SaveImagePgm((char*)"",(char*)"Graphe",Graph2D,length,width); //Pour sauvegarder l'image
 printf("\n\n Pour quitter,appuyer sur la barre d'espace");
 fflush(stdout);

 //boucle d'evenements
  for(;;)
     {
      XNextEvent(display,&ev);
       switch(ev.type)
        {
	 case Expose:   

         XPutImage(display,win_ppicture,gc,x_ppicture,0,0,0,0,x_ppicture->width,x_ppicture->height);  
         break;

         case KeyPress: 
         XDestroyImage(x_ppicture);

         XFreeGC(display,gc);
         XCloseDisplay(display);
         flag_graph=0;
         break;
         }
   if (!flag_graph) break;
   }
 } 
       
 //retour sans probleme 
 printf("\n Fini... \n\n\n");
 return 0;
 }
 