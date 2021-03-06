#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
//windows
#include <Windows.h>
#else
//Linux
#include <time.h>
#endif

extern void mandelbrot(unsigned int r_start, unsigned int r_end, unsigned int i_start, unsigned int i_end, unsigned int res, unsigned char *img);

void createBMP(unsigned char* image_data, int w, int h) {
	FILE *f;

	unsigned char bmpfileheader[14] = { 'B','M', 0,0,0,0, 0,0,0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
	unsigned char bmpcontent[3] = { 255,0,0 };
	unsigned char bmppad[1] = { 0 };

	int padSize = (4 - 3 * w % 4) % 4;
	int sizeData = w*h * 3 + h*padSize;
	int filesize = sizeData + sizeof(bmpfileheader) + sizeof(bmpinfoheader);

	/* Construct header with filesize part */
	bmpfileheader[2] = (unsigned char)(filesize);
	bmpfileheader[3] = (unsigned char)(filesize >> 8);
	bmpfileheader[4] = (unsigned char)(filesize >> 16);
	bmpfileheader[5] = (unsigned char)(filesize >> 24);

	bmpinfoheader[4] = (unsigned char)(w);
	bmpinfoheader[5] = (unsigned char)(w >> 8);
	bmpinfoheader[6] = (unsigned char)(w >> 16);
	bmpinfoheader[7] = (unsigned char)(w >> 24);
	bmpinfoheader[8] = (unsigned char)(h);
	bmpinfoheader[9] = (unsigned char)(h >> 8);
	bmpinfoheader[10] = (unsigned char)(h >> 16);
	bmpinfoheader[11] = (unsigned char)(h >> 24);

	bmpfileheader[20] = (unsigned char)(sizeData);
	bmpfileheader[21] = (unsigned char)(sizeData >> 8);
	bmpfileheader[22] = (unsigned char)(sizeData >> 16);
	bmpfileheader[23] = (unsigned char)(sizeData >> 24);

	f = fopen("baked_Mandelbrot.bmp", "wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);


	//jede reihe muss ein vielfaches von 4 sein, beim speichern 1 reihe in den buffer schreiben
	//anschlie�end �ber reihe iterieren und schreiben und mit paddding auff�llen

	//gesamtanzahl an sample punkten = l�nge array
	//xRes = width, yRes = height

	// array in assemblee von oben links nach unten rechts bef�llen
	int i, j;
	for (i = w*(h - 1); i >= 0; i = i - w) {
		//i gibt startpunkt in array

		//j iteriet �ber die Zeile
		for (j = 0; j<w; j++) {
			bmpcontent[0] = (int)(255 * (image_data[i + j] - 48) / (float)9);
			bmpcontent[1] = (int)(255 * (image_data[i + j] - 48) / (float)9);
			bmpcontent[2] = (int)(255 * (image_data[i + j] - 48) / (float)9);
			fwrite(bmpcontent, 1, 3, f);
		}

		//insert padding

		//bmppad evtl auf 1 anh�ngen
		for (int x = padSize; x>0; x--) {
			fwrite(bmppad, 1, 1, f);
		}
	}



	fclose(f);
}

//returns a timestamp with sub seconde precision
double secondes()
{
#ifdef _MSC_VER
	static LARGE_INTEGER frequency;
	if (frequency.QuadPart == 0)
		QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return (now.QuadPart / (double)frequency.QuadPart);
#else
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec + now.tv_nsec / 1000000000.0;
#endif
}

//checked den String auf fehlerhafte eingaben buchstaben oder zeichen nicht zwischen 0 und 9
int invalidInput(char* input) {
	//find out length
	int length = 0;
	while (input[length] != '\0') {
		length++;
	}


	//check for invalid character
	for (int i = 0; i<length; i++) {
		if (input[i] < '0' || input[i] > '9') {
			return 1;
		}
	}

	if (length>1 && input[0] == '0') {
		return 1;
	}
	return -1;
}

int main(int argc, char* argv)
{
	/*
	* Parsing commandline arguments and checking for valid input
	* @param r_Start, r_End: Realteil start und Ende
	* @param i_STart, i_End: Imagin�rteil Start und Ende
	* @param resolution: Aufl�sung des Bildes am Ende
	*/
	int r_Start=0, r_End=0, i_Start=0, i_End=0, resolution=0;

	//Kommandozeilen Argumente Anzahl checken bei flascher Anzahl beenden.
	if (argc == 6) {
		/*starting time tracking*/
		double time1, time2;
		time1 = secondes();

		/*
		 * Parsing command line arguments
		 */
		printf("parsing started\n");
		
		for (int i = 1; i<6; i++) {

			if (invalidInput(&argv[i]) == 1) {
				printf("\nFehlerhafte Eingabe. Bitte geben sie ein /main.c int int int int int. \nProgramm beendet \nErste Stelle");
				return -1;
			}
		}

		r_Start = atoi(&argv[1]);
		r_End = atoi(&argv[2]);
		i_Start = atoi(&argv[3]);
		i_End = atoi(&argv[4]);
		resolution = atoi(&argv[5]);

		printf("Input of Program:\nr_Start: \t%d\nr_End: \t\t%d\ni_Start: \t%d\ni_End: \t\t%d\nresolution: \t%d\n", r_Start, r_End, i_Start, i_End, resolution);

		printf("Parsing done\n");

		/*
		 * Starting main task
		 */
		unsigned char* image = malloc(sizeof(unsigned char)*resolution*resolution) ;
		/*
		* Filling image in assembler
		*/
		printf("Starting to calculate Image\n");
		mandelbrot(r_Start, r_End, i_Start, i_End, resolution, image);
		printf("Ended to calculate Image\n");

		/*
		 * creating the bmp file
		 */
		printf("Starting to write Data to File\n");
		createBMP(image, resolution, resolution);
		free(image);
		printf("Ended to write Data to File\n");

		/*ending time tracking printing time*/
		time2 = secondes();
		double duration = time2 - time1;
		printf("%f\t,%f,\t%f", duration, time2, time1);

		getchar();
	}
	return 0;
}
