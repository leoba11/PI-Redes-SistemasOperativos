#include <string>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <unistd.h>

using namespace std;

void lector();
int main(int argc, char **argv){
   lector();
}



void lector(){
	char* rut;
	const char* ruta;
	int contador=0;
	int tam=0;
    int f=0;
    int cont_imagen=0;
    bool esImagen=false;
    string ruta_cont;
    string exten;
	vector<string> rutas;
	string fileName;
	DIR* pDIR;
	string ruta_aux;
	int formato=0;
	int seguir=1;
   

   
        cout<<"Digite 1 para una nueva ruta o 0 para terminar "<<endl;
        cin >> seguir;
	
    
	while(seguir){

		 cout<<"Digite 1 para formtato png \n 2 para formato gif \n 3 para formatio jpg"<<endl;
         cin >> formato;
		 switch(formato) {
            case 1 : exten="gnp.";
               break;       
            case 2 : exten="fig.";
               break;
            case 3 : exten="gpj.";
               break;
    }
			cout<<"Digite la ruta"<<endl;
        cin >> rut;
        ruta=rut;



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
	     
	     /*while(rutas.size() != 0 ){
	     	
	     	if(contador < 2){
	     		contador++;
	     		cont_imagen+=2;
	     		ruta_cont = rutas.back();
	     		rutas.pop_back();
	     	
	     		if(fork() == 0){
	    			//contratista(ruta_cont, contador_imagen);
	     		}
	     	}
	    }*/
    }

    cout<<"Digite 1 para una nueva ruta o 0 para terminar "<<endl;
        cin >> seguir;
		


}


	

}




