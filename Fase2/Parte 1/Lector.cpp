#include <string>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>

using namespace std;

void lector(const char* ruta);
int main(int argc, char **argv){
   lector("/home/royerchq/Downloads/prueba");
}



void lector(const char* ruta){
	int contador=0;
	int tam=0;
    int f=0;
    int cont_imagen=0;
    bool esImagen=false;
    string ruta_cont;
    string exten="gnp.";
	vector<string> rutas;
	string fileName;
	DIR* pDIR ;
	string ruta_aux;
	
	if(pDIR = opendir(ruta)){
        while(dirent* entry = readdir(pDIR)) {
            fileName = entry->d_name;

         
        tam = fileName.size()-4;
        f=0;
        cont_imagen=0;
        esImagen=false;
        for(int i= fileName.size()-1; i >= tam ; i--){
        	
        	if(fileName[i]==exten[f]){
        	
        		esImagen=true;
        		cont_imagen++;
        	
        	}else{
        		
        		esImagen=false;

        	}
        	
        	f++;
        }
       
        if(esImagen && cont_imagen==4){
        	ruta_aux=ruta;
        	rutas.push_back(ruta_aux+"/"+fileName);
        	cout << fileName << '\n';
        }
   }

    closedir(pDIR);
     
     while(rutas.size() != 0 ){
     	
     	if(contador < 2){
     		contador++;
     		//ruta_cont = rutas.back();
     		//rutas.pop_back();
     	
     		if(fork() == 0){
     			//scontador++;
     			cout<<"hijo"<<contador<<endl;
    			//contratista(ruta_cont, &contador);
     		    ruta_cont = rutas.back();
     		    rutas.pop_back();
     	

     		}	
     	}

     }
 }

}




