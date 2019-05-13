#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"  //pour cvLoadimage , iplimage ...etc.
#include <opencv2/core/types_c.h> 		// pour le namespace "cv"
#include <opencv2/core/core_c.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <stddef.h>
#include <algorithm>
#include <cstdlib>

// les deux images 
const char * const IMAGE_INITIALE = "Initial";
const char * const IMAGE_FINALE = "Final colored image";

using namespace cv;
using namespace std;

Mat initial_image;

// table d'équivalence 
int** table_equivalence = new int*;

// on va relier les pixels 
class RelatePixel {
	
	private:
    char value;
    int label;
    RelatePixel * vUp;
    RelatePixel * vLeft;

public:
	
	// constructeur 
    RelatePixel(int value, RelatePixel * upneighbour, RelatePixel * leftneighbour);

    // le mapping
    static RelatePixel*** imageToRelatePixelArray(Mat image);
    static Mat RelatePixelArrayToImage(RelatePixel** pixels, int nbRows, int nbCol);
    
    // les getters et setters 
    void SetLabel(int label);
    int GetLabel() const;
    RelatePixel* GetVGauche() const;
    RelatePixel* GetVHaut() const;
    char GetValeur() const;
    void SetValeur(char value);
    
    //Utils
    bool isNotNull(){
	if (value  && label  && vUp  && vLeft  ) return false;
    else return true;
	}
    ostream& operator<<(ostream &flux);
};

RelatePixel::RelatePixel(int color, RelatePixel * upneighbour, RelatePixel * leftneighbour){
 this->value = color;
 this->vUp = upneighbour;
 this->vLeft = leftneighbour;
 this->label = -1;
}

char RelatePixel::GetValeur() const {
    return value;
}

void RelatePixel::SetValeur(char value) {
    this->value = value;
}

RelatePixel* RelatePixel::GetVGauche() const {
    return vLeft;
}

RelatePixel* RelatePixel::GetVHaut() const {
    return vUp;
}

void RelatePixel::SetLabel(int label) {
    this->label = label;
    this->value = 0;
}

int RelatePixel::GetLabel() const {
    return label;
}

 /**
 * Convertit une image en un tableau de RelatePixel
 * @param image l'image à modifier
 * @return le tableau de RelatePixel correspondant à l'image
 */
RelatePixel*** RelatePixel::imageToRelatePixelArray(Mat image){
    cout<<"imageToRelatePixelArray begins"<<endl;
    int nbCol = image.cols;
    int nbLigne = image.rows;  
    RelatePixel*** pixels = new RelatePixel**[nbLigne];
    
    int i,j;
    RelatePixel * up,* left;
    
    for (i=0; i<nbLigne; i++){
        pixels[i] = new RelatePixel*[nbCol];
        for (j=0; j<nbCol; j++){
            //first line (not neighbour up)
            if (i == 0) {
                up = NULL;
            } else {
                up = pixels[i-1][j];
            }
            
            //first column (not neighbour left)
            if (j == 0) {
                left = NULL;
            } else {
                left = pixels[i][j-1];
            }
            //Créate pixel
            pixels[i][j] = new RelatePixel(image.at<uchar>(Point(i,j)), up, left);
        }
    }
    cout<<"fin création imageToRelatePixelArray"<<endl;
    return pixels;
}

Mat RelatePixel::RelatePixelArrayToImage(RelatePixel** pixels, int nbRows, int nbCol){
	cv::Mat img(nbRows, nbCol, CV_8UC3);
    cout<<"RelatePixelArrayToImage begins"<<endl;
    int i,j,offset;
    for (i=0; i<nbRows ; i++){
        offset = i*nbCol;
        for(j=0; j<nbCol; j++){
         //img.ptr<cv::Vector>(j+offset) = pixels[i][j].GetValeur();
        }
    }
    cout<<"RelatePixelArrayToImage ends"<<endl;
    return img;
}


ostream& RelatePixel::operator <<(ostream& flux){ 
    flux<<"Pixel : {value: "<<this->GetValeur()
            <<", e: "<<this->GetLabel()
            <<", vUp: "<<this->GetVHaut()
            <<", vGau: "<<this->GetVGauche();
    return flux;    
}

bool isValidCommand(int nbArgs, char** args);
inline Mat getLabeling();
RelatePixel*** algoLabeling(RelatePixel*** pixels);

/** algo vu en cours  */
RelatePixel*** algoLabeling(RelatePixel*** pixels) {
    cout<<"algoLabeling begins"<<endl;
    int nbLigne = initial_image.rows;
    int nbCol = initial_image.cols;

    RelatePixel* pixel, * vup,  * vleft;

    int currentEtiq = 0, eh, eg;
    int i, j;
    //loop table RelatePixels
    for (i = 0; i < nbLigne; i++) {
        for (j = 0; j < nbCol; j++) {
            pixel = pixels[i][j];
            //if pixel
            if (pixel->GetValeur() > 250) {

            } else {
                //get  neighbours up and left
                vup = pixel->GetVHaut();
                vleft = pixel->GetVGauche();

                //if  pixel has 2 neighbours
                if (vup->isNotNull() && vleft->isNotNull()) {
                    eh = vup->GetLabel();
                    eg = vleft->GetLabel();

                    //if the two neighbours have the same label 
                    if (eh == eg) {
                        if (eh == -1) { // labels  nulles
                            pixel->SetLabel(currentEtiq++);
                        } else {
                            pixel->SetLabel(eh);
                        }
                    }
                    //if the two neighbours have different label 
                    else {
                        // neighbour up has no label
                        if (eh == -1) {
                            pixel->SetLabel(eg);
                        }
                        // neighbour left has no label
                        else if (eg == -1) {
                            pixel->SetLabel(eh);
                        } 
                        // labels différentes
                        else {
                            pixel->SetLabel(min(eh,eg));
                            //TODO equiv
                        }
                    }
                }//if  pixel has a least a neighbour null
                else {
                    // 2 neighbours are null
                    if (!vup->isNotNull() && !vleft->isNotNull()) {
                        pixel->SetLabel(currentEtiq++);
                    }// one of two is null
                    else {
                        RelatePixel* neighbour;
                        if (vup->isNotNull()) {
                            neighbour = vup;
                        } else {
                            neighbour = vleft;
                        }
                        //verify label  neighbour
                        eh = neighbour->GetLabel();
                        if (eh >= 0){
                            pixel->SetLabel(eh);
                        } else {
                            pixel->SetLabel(currentEtiq++);
                        }
                    }
                }
            }
        }
    }
    cout<<"algoLabeling ends, currentEtiq = "<<currentEtiq<<endl;
    return pixels;
}

Mat getLabeling() {
    cout << "étiquettage commence" << endl;
    RelatePixel*** pixels = RelatePixel::imageToRelatePixelArray(initial_image);
    pixels = algoLabeling(pixels);
    // img = RelatePixel::RelatePixelArrayToImage(pixels, initial_image->height, initial_image->width);
   //Mat img.data = NULL;
   Mat img;
    cout << "fin étiquettage" << endl;
return img;
}


/** Verifie la validité des arguments
 * @param argc le nombre d'arguments passés
 * @param argv les arguments */
int main(int argc, char** argv) {
    //verification des arguments du proramme
    if ( argc != 2 ) {
        printf( "Usage: display_image ImageToLoadAndDisplay\n" );
        return -1;
    }
    
    Mat tmp = cv::imread(argv[1],CV_LOAD_IMAGE_GRAYSCALE); /* Read the file */
    
    if ( !tmp.data ) { /* Check for invalid input */
        printf("Could not open or find the image\n") ;
        return -1;
    }
    
	//	tmp is copied to initial_image
   tmp.copyTo(initial_image);
   
    //initialisation des fenetres
    cvNamedWindow(IMAGE_INITIALE);
    cvNamedWindow(IMAGE_FINALE);

    //Affichage des images
    imshow(IMAGE_INITIALE, initial_image);
    imshow(IMAGE_FINALE, getLabeling());

    //Attente d'une entrée clavier
    waitKey(0);

    return 0;
}
