#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// structura pentru reprezentarea unei imagini ce retine dimensiunile,
// val maxima a culorii, formatul, pointer la continutul imaginii
typedef struct {
	int width;
	int height;
	int max_val;
	char pvalue[3]; // valoarea formatului
	unsigned char *data;
	int selected;       // ma ajuta sa retin daca o zona este selectata
	int x1, y1, x2, y2; // daca o zona e selectata, acestea reprezinta
	// coordonatele colturilor de stanga, sus si dreapta, jos
	int loaded; // cu loaded retin daca o imagine e incarcata sau nu
	int appliedall;
	int selectedforrotate;
} image;

// functie ce elibereaza memoria
void free_img(image *img)
{
	if (img->data) {
		free(img->data);
		img->data = NULL;
	}
}

// functie care verifica daca a fost incarcata o imagine
// am folosit o la inceputul fiecarei operatii
int isloaded(const image *img)
{
	if (!img->data) {
		printf("No image loaded\n");
		return 0;
	}
	return 1;
}

// functie ce aloca memorie pentru imagini in format p2 si p5
int p2_p5_memalloc(image *img)
{
	img->data = (unsigned char *)malloc(img->width * img->height);
	if (!img->data)
		return 0;
	return 1;
}

// functie ce aloca memorie pentru imagini in format p3 si p6
// functiile returneaza 0 daca alocarea esueaza
int p3_p6_memalloc(image *img)
{
	// in cazul alocarii de memorie pentru imaginile color, aloc
	// de 3 ori mai multa memorie, fiecare pixel avand 3 valorii
	// pentru fiecare culoare (RGB)
	img->data = (unsigned char *)malloc(img->width * img->height * 3);
	if (!img->data)
		return 0;
	return 1;
}

// functie care ma ajuta sa limiteza o valoare in intervalul [0,255]
double clampp(double value)
{
	if (value < 0)
		return 0;
	if (value > 255)
		return 255;
	return value;
}

// functie ce incarca o imagine
int load(image *img, const char *filename)
{
	free_img(img); // eliberez memoria unei imagini incarcate precedent
	FILE *file = fopen(filename, "rb"); // deschid fisierul pentru citire
	if (!file) {
		printf("Failed to load %s\n", filename);
		return 0;
	}
	// citesc primele date din fisier
	if (!fgets(img->pvalue, sizeof(img->pvalue), file) ||
		fscanf(file, "%d%d%d", &img->width, &img->height, &img->max_val) != 3) {
		fclose(file);
		printf("Failed to load %s\n", filename);
		return 0;
	}
	int verif = 1; // ma ajuta sa verific pe parcurs erorile
	// incep sa alaoc memorie si sa citsc datele din imagini
	if (strncmp(img->pvalue, "P2", 2) == 0) {
		verif = p2_p5_memalloc(img);
		for (int i = 0; verif && i < img->width * img->height; i++) {
			int pixel;
			if (fscanf(file, "%d", &pixel) != 1) {
				verif = 0;
				break;
			}
			img->data[i] = (unsigned char)pixel;
		}
	} else if (strncmp(img->pvalue, "P3", 2) == 0) {
		verif = p3_p6_memalloc(img);
		for (int i = 0; verif && i < img->width * img->height * 3; i++) {
			if (fscanf(file, "%hhu", &img->data[i]) != 1) {
				verif = 0;
				break;
			}
		}
	} else if (strncmp(img->pvalue, "P5", 2) == 0) {
		verif = p2_p5_memalloc(img);
		fscanf(file, " ");
		// sar peste spatiile goale
		if (fread(img->data, sizeof(unsigned char), img->width * img->height,
				  file) != (size_t)img->width * img->height && verif) {
			verif = 0;
		}
	} else if (strncmp(img->pvalue, "P6", 2) == 0) {
		verif = p3_p6_memalloc(img);
		fscanf(file, " ");
		if (fread(img->data, sizeof(unsigned char),
				  img->width * img->height * 3, file) !=
				  (size_t)img->width * img->height * 3 && verif) {
			verif = 0;
		}
	} else {
		verif = 0;
	}
	// inchid fisierul
	fclose(file);

	if (verif) {
		printf("Loaded %s\n", filename);
		return 1;
	}
	free_img(img);
	printf("Failed to load %s\n", filename);
	return 0;
	// afisez mesajele corespunzatoare, folosindu-ma de variabila verif
}

// functie pentru interschimbarea a doua variabile
void swappp(int *a, int *b)
{
	int aux = *a;
	*a = *b;
	*b = aux;
}

void selectcoord(image *img, int x1, int y1, int x2, int y2)
{
	if (!isloaded(img))
		return;
	if (x2 < x1)
		swappp(&x1, &x2);
	if (y2 < y1)
		swappp(&y1, &y2);
	// verific daca coordonatele sunt corecte
	if (x1 < 0 || x2 > img->width || y1 < 0 || y2 > img->height ||
		(x1 == x2 && x2 == y1 && y1 == y2) /* daca toate 4 sunt
		egale intre ele */|| x1 >= x2 || y1 >= y2) {
		printf("Invalid set of coordinates\n");
		img->selectedforrotate = 0;
		return;
	}
	// actualizez coordonatele
	img->x1 = x1;
	img->y1 = y1;
	img->x2 = x2;
	img->y2 = y2;
	img->selected = 1;
	img->selectedforrotate = 1;
	printf("Selected %d %d %d %d\n", x1, y1, x2, y2);
}

// functie pentru selectarea imaginii complete
void select_all(image *img)
{
	if (!isloaded(img))
		return;
	img->x1 = 0;
	img->y1 = 0;
	img->x2 = img->width;
	img->y2 = img->height;
	img->selected = 1;
	img->selectedforrotate = 1;
	printf("Selected ALL\n");
}

// functie pentru modificarea imaginii pe dimensiunile selectate
void crop(image *img)
{
	if (!isloaded(img))
		return;
	// daca nu este selectatea vreo regiune, se considera toata imaginea
	if (!img->selected) {
		img->x1 = 0;
		img->y1 = 0;
		img->x2 = img->width;
		img->y2 = img->height;
	}
	// calculez noile dimensiuni
	int width_updated = img->x2 - img->x1, height_updated = img->y2 - img->y1,
			previous_index, index_updated;
	unsigned char *data_updated; // pointer pentru noile date ale imaginii
	if (strncmp(img->pvalue, "P3", 2) == 0 ||
		strncmp(img->pvalue, "P6", 2) == 0) {
		// aloc memorie pt imaginile RGB
		data_updated =
		(unsigned char *)malloc(width_updated * height_updated * 3);
		for (int i = img->y1; i < img->y2; i++) {
			for (int j = img->x1; j < img->x2; j++) {
				for (int RGB = 0; RGB < 3; RGB++) {
					// inlocuiesc pixelii in noua zona de memorie
					previous_index = (i * img->width + j) * 3 + RGB;
					index_updated =
					((i - img->y1) * width_updated + (j - img->x1)) * 3 + RGB;
					data_updated[index_updated] = img->data[previous_index];
				}
			}
		}
	} else {
		// aloc memorie pt imaginile p2, p5
		data_updated = (unsigned char *)malloc(width_updated * height_updated);
		for (int i = img->y1; i < img->y2; i++) {
			for (int j = img->x1; j < img->x2; j++) {
				previous_index = i * img->width + j;
				index_updated = (i - img->y1) * width_updated + (j - img->x1);
				data_updated[index_updated] = img->data[previous_index];
			}
		}
	}
	free(img->data); // eliberez memoria
	img->data = data_updated;
	img->width = width_updated;
	img->height = height_updated; // actualizez noile dimensiuni
	img->x1 = 0;
	img->y1 = 0;
	img->x2 = width_updated;
	img->y2 = height_updated;
	// img->selectedforrotate = 0;
	printf("Image cropped\n");
}

// functie ce verifica daca un numar este putere de a lui 2
int power2verif(int n)
{
	while (n != 1) {
		if (n % 2 != 0)
			return 0;
		n /= 2;
	}
	return 1;
}

// functie pentru calcularea histogramei
void histogram(const image *img, int asteriks, int bins)
{
	asteriks = 0;
	if (!isloaded(img))
		return;
	if (strncmp(img->pvalue, "P3", 2) == 0 ||
		strncmp(img->pvalue, "P6", 2) == 0) {
		printf("Black and white image needed\n");
		return;
	}
	if (!power2verif(bins)) {
		printf("Invalid set of parameters\n");
		return;
	}
	int hist[bins];
	for (int i = 0; i < bins; ++i)
		hist[i] = 0;
	for (int i = 0; i < img->width * img->height; i++)
		hist[img->data[i]]++;
	int max_vall = 0;
	for (int i = 0; i < bins; i++) {
		if (hist[i] > max_vall)
			max_vall = hist[i];
	}
	for (int i = 0; i < bins; i++) {
		printf("%3d | ", i);
		int nrofasteriks = (hist[i] * asteriks) / max_vall;
		for (int j = 0; j < nrofasteriks; j++)
			printf("*");
		printf("\n");
	}
}

// functie pentru egalizarea unei imagini
void equalize2(image *img)
{
	if (!isloaded(img))
		return;
	if (strncmp(img->pvalue, "P2", 2) != 0 &&
		strncmp(img->pvalue, "P5", 2) != 0) {
		printf("Black and white image needed\n");
		return; // verific daca este imagine alb-negru
	}
	// selectez intreaga imagine daca nu a fost selectata vreo regiune inainte
	if (!img->selected) {
		img->x1 = 0;
		img->y1 = 0;
		img->x2 = img->width;
		img->y2 = img->height;
	}
	// consider hist un vector de frecventa si construiesc histograma
	double hist[256] = {};
	double selected_pixels = img->width * img->height;
	for (int y = 0; y < img->height; y++) {
		for (int x = 0; x < img->width; x++) {
			int index = y * img->width + x;
			hist[img->data[index]]++;
		}
	}
	double s = 0;
	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			int index = i * img->width + j;
			int valindex = img->data[index];
			for (int k = 0; k <= valindex; k++)
				s = s + hist[k];
			// conform formulei, atribui ficerui pixel valorea corespunzatoare
			img->data[index] =
			clampp(round((double)((255 * s) / selected_pixels)));
			s = 0;
		}
	}
	printf("Equalize done\n");
}

// functie care salveaza imaginea intr-un fisier, format ascii sau binar
void save_image(image *img, const char *filename, int ascii)
{
	if (!isloaded(img))
		return;
	FILE *file = fopen(filename, "w"); // deschid fisierul cu modul writing
	if (!file) {
		printf("Failed to save %s\n", filename);
		return;
	}
	char format[3]; // determin formatul in care salvez fisierul
	strcpy(format, img->pvalue);
	if (ascii) { // daca trebuie salvat ascii, modific pvalue
		if (strncmp(img->pvalue, "P5", 2) == 0)
			strcpy(format, "P2");
		else if (strncmp(img->pvalue, "P6", 2) == 0)
			strcpy(format, "P3");
	}
	// apoi pot scrie antetul fisierului si datele imaginii
	fprintf(file, "%s\n%d %d\n%d\n", format, img->width, img->height,
			img->max_val);
	int nrpixeli = 0;
	if (strncmp(format, "P2", 2) == 0) {
		// pentru p2 pixelii se salveaza in format ascii
		for (int i = 0; i < img->width * img->height; i++) {
			fprintf(file, "%d ", img->data[i]);
			nrpixeli++; // retin cati sunt pe fiecare rand
			// pentru a trece pe urmatorul
			if (nrpixeli == img->width)
				fprintf(file, "\n");
		}
		// pentru p2 si p5 trebuie sa fie atatia pixeli cat img->width,
		// iar pentru p3 si p6 de 3 ori mai multi, intrucat sunt imagini
		// format RGB, fiecare pixel fiind alcatuit din 3 valori
	} else if (strncmp(format, "P3", 2) == 0) {
		for (int i = 0; i < img->width * img->height * 3; i++) {
			fprintf(file, "%d ", img->data[i]);
			nrpixeli++;
			if (nrpixeli == img->width * 3) {
				fprintf(file, "\n"); // trec pe randul urmator
				nrpixeli = 0; // resetez contorul
			}
		}
	} else if (strncmp(format, "P5", 2) == 0) {
		fwrite(img->data, sizeof(unsigned char), img->width * img->height,
			   file);
	} else if (strncmp(format, "P6", 2) == 0) {
		fwrite(img->data, sizeof(unsigned char), img->width * img->height * 3,
			   file);
	}
	fclose(file); // inchid fisierul
	printf("Saved %s\n", filename);
	// img->selectedforrotate = 0;
}

// functie cu ajutorul careia impart suma calculata in functie de operatie
int divide(int sum, const char *kernel_type)
{
	if (strcmp(kernel_type, "BLUR") == 0)
		sum = sum / 9;
	if (strcmp(kernel_type, "GAUSSIAN_BLUR") == 0)
		sum = sum / 16;
	return clampp(sum);
}

// functie care calculeaza noua valoare a pixelului, dupa aplicarea kernelului
// corespunzator
int spixel(image *img, int x, int y, int RGB, int kernel[3][3],
		   const char *kernel_type)
{
	int s = 0;
	// variabila RGB ia pe rand valorile 0, 1, 2 pentru pozitia pixeului
	// corespunzator fiecarei culori astfel, pentru y=y-1, adun pixelii de pe
	// prima linie inmultiti cu valoarea corespunzatoare din kernel
	y--;
	s += img->data[(y * img->width + (x - 1)) * 3 + RGB] * kernel[0][0] +
			img->data[(y * img->width + x) * 3 + RGB] * kernel[0][1] +
			img->data[(y * img->width + (x + 1)) * 3 + RGB] * kernel[0][2];
	// apoi, y ia valoarea initiala si parcurg pixelii de pe urmatorul rand, cel
	// din mijloc
	y++;
	s += img->data[(y * img->width + (x - 1)) * 3 + RGB] * kernel[1][0] +
			img->data[(y * img->width + x) * 3 + RGB] * kernel[1][1] +
			img->data[(y * img->width + (x + 1)) * 3 + RGB] * kernel[1][2];
	// in final, parcurg pixelii de pe randul de jos
	y++;
	s += img->data[(y * img->width + (x - 1)) * 3 + RGB] * kernel[2][0] +
			img->data[(y * img->width + x) * 3 + RGB] * kernel[2][1] +
			img->data[(y * img->width + (x + 1)) * 3 + RGB] * kernel[2][2];
	return divide(s, kernel_type);
}

// functie care aplica kernelul pe intreaga imagine
void matrixmultiplication(image *img, int kernel[3][3],
						  const char *kernel_type)
{
	// daca nu este nicio imagine selectata, consider toata imaginea
	if (!img->selected) {
		img->x1 = 0;
		img->y1 = 0;
		img->x2 = img->width;
		img->y2 = img->height;
	}
	// consider aux o zona auxiliara de memorie in care stochez datele imaginii
	// dupa modificare
	unsigned char *aux = (unsigned char *)malloc(img->width * img->height * 3);
	for (int i = 0; i < img->width * img->height * 3; i++)
		aux[i] = img->data[i];
	// dupa ce ii aloc memorie, retin datele din imaginea initiala
	// pentru a putea pastra conturul imaginii nemodificat
	for (int y = img->y1; y < img->y2; y++) {
		if (y != 0 && y != img->height - 1) {
			// sar peste marginea imaginii cu ajutorul if-urilor
			for (int x = img->x1; x < img->x2; x++) {
				if (x != 0 && x != img->width - 1) {
					for (int RGB = 0; RGB < 3; RGB++) {
						// calculez noua valoare cu ajutorul functiei
						//  si o retin in zona auxiliara
						int new_val =
						spixel(img, x, y, RGB, kernel, kernel_type);
						aux[(y * img->width + x) * 3 + RGB] = new_val;
					}
				}
			}
		}
	}
	//inlocuiesc datele din imaginea initiala cu cele din cea auxiliara
	for (int i = 0; i < img->width * img->height * 3; i++)
		img->data[i] = aux[i];
	// eliberez memoria auxiliara
	free(aux);
}

// functie pentru optiunea APPLY
void apply(image *img, const char *parameter)
{
	if (!isloaded(img))
		return;
	if (strncmp(img->pvalue, "P2", 2) == 0 ||
		strncmp(img->pvalue, "P5", 2) == 0) {
		printf("Easy, Charlie Chaplin\n");
		return; // mesajul corespunzator imaginilor alb-negru
	}
	if (!parameter) {
		printf("Invalid command\n");
	} else {
		// initializez kernelurile corespunzatoare pentru fiecare operatie
		int kernel_EDGE[3][3] = {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}};
		int kernel_SHARPEN[3][3] = {{0, -1, 0}, {-1, 5, -1}, {0, -1, 0}};
		int kernel_BLUR[3][3] = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
		int kernel_GAUSSIAN_BLUR[3][3] = {{1, 2, 1}, {2, 4, 2}, {1, 2, 1}};
		// verific parametrul si aplic kernelul la fiecare
		if (strcmp(parameter, "EDGE") == 0) {
			matrixmultiplication(img, kernel_EDGE, "EDGE");
			printf("APPLY EDGE done\n");
		} else if (strcmp(parameter, "SHARPEN") == 0) {
			matrixmultiplication(img, kernel_SHARPEN, "SHARPEN");
			printf("APPLY SHARPEN done\n");
		} else if (strcmp(parameter, "BLUR") == 0) {
			matrixmultiplication(img, kernel_BLUR, "BLUR");
			printf("APPLY BLUR done\n");
		} else if (strcmp(parameter, "GAUSSIAN_BLUR") == 0) {
			matrixmultiplication(img, kernel_GAUSSIAN_BLUR, "GAUSSIAN_BLUR");
			printf("APPLY GAUSSIAN_BLUR done\n");
		} else {
			printf("APPLY parameter invalid\n");
		}
	}
}

void rotate_at_90(image *img)
{
	int selected_width;
	int selected_height;
	if (img->selectedforrotate) {
		selected_width = img->x2 - img->x1;
		selected_height = img->y2 - img->y1;
	} else {
		selected_width = img->width;
		selected_height = img->height;
	}
	unsigned char *rotated_imgdata;
	if (strncmp(img->pvalue, "P2", 2) == 0 ||
		strncmp(img->pvalue, "P5", 2) == 0) {
		rotated_imgdata =
		(unsigned char *)malloc(selected_width * selected_height);
		for (int i = 0; i < selected_height; i++) {
			for (int j = 0; j < selected_width; j++) {
				int original_idx = (i + img->y1) * img->width + (j + img->x1);
				int rotated_idx =
				(selected_width - 1 - j) * selected_height + i;
				rotated_imgdata[rotated_idx] = img->data[original_idx];
			}
		}
	} else {
		rotated_imgdata =
		(unsigned char *)malloc(selected_width * selected_height * 3);
		for (int i = 0; i < selected_height; i++) {
			for (int j = 0; j < selected_width; j++) {
				for (int RGB = 0; RGB < 3; RGB++) {
					int original_idx =
					((i + img->y1) * img->width + (j + img->x1)) * 3 + RGB;
					int rotated_idx =
					((selected_width - 1 - j) * selected_height + i) * 3 + RGB;
					rotated_imgdata[rotated_idx] = img->data[original_idx];
				}
			}
		}
	}
	free(img->data);
	img->data = rotated_imgdata;
	if (img->selectedforrotate) {
		img->x2 = img->x1 + selected_height;
		img->y2 = img->y1 + selected_width;
	} else {
		swappp(&img->width, &img->height);
	}
}

void rotate(image *img, int degrees)
{
	if (!isloaded(img))
		return;
	int aux_degrees = degrees;
	if (degrees < 0)
		degrees += 360;
	if (degrees != 90 && degrees != 180 && degrees != 0 &&
		degrees != 270 && degrees != 360) {
		printf("Unsupported rotation angle\n");
		return;
	}
	if (!img->selectedforrotate) {
		if (img->x2 - img->x1 != img->y2 - img->y1) {
			printf("The selection must be square\n");
			return;
		}
	} else {
		img->x1 = 0;
		img->y1 = 0;
		img->x2 = img->width;
		img->y2 = img->height;
	}
	if (aux_degrees == 0) {
		printf("Rotated 0\n");
		return;
	}
	degrees = (degrees / 90) % 4;
	for (int i = 0; i < degrees; i++)
		rotate_at_90(img);
	printf("Rotated %d\n", aux_degrees);
}

void process_histogram_command(const image *img,
							   const char *arguments, int image_loaded)
{
	int x, y;
	char extra;
	if (image_loaded) {
		if (arguments) {
			int num = sscanf(arguments, "%d %d %c", &x, &y, &extra);
			if (num == 2)
				histogram(img, x, y);
			else
				printf("Invalid command\n");
		} else {
			printf("Invalid command\n");
		}
	} else {
		printf("No image loaded\n");
	}
}

void process_save_command(image *img, const char *arguments)
{
	char filename[30];
	int ascii = 0;
	if (arguments) { // verific daca comanda contine "ascii"
		if (strstr(arguments, "ascii"))
			ascii = 1;
		if (sscanf(arguments, "%s", filename) == 1)
			save_image(img, filename, ascii);
		else
			printf("No file name provided for SAVE command\n");
	} else {
		printf("No file name provided for SAVE command\n");
	}
}

int main(void)
{
	image img = {};
	char input[60], *command, *arguments;//pointeri pentru comanda si argumente
	int command_found, image_loaded = 0;// lista de comenzi valide
	char const *command_list[] = {"LOAD",  "SELECT", "HISTOGRAM",
		"SAVE",  "EQUALIZE", "CROP", "APPLY", "EXIT", "ROTATE"};
	while (1) {
		command_found = 0;
		fgets(input, sizeof(input), stdin); // sterg caracterul nul din input
		char *newline = strchr(input, '\n');
		if (newline)
			*newline = '\0'; // sparg inputul cu ajutorul lui strtok
		command = strtok(input, " ");
		arguments = strtok(NULL, "");
		for (int i = 0; i < 9; i++) {
			if (strcmp(command, command_list[i]) == 0) {
				command_found = 1;
				break;
			}
		}
		if (command_found) {
			if (strcmp(command, "LOAD") == 0) {
				if (arguments)
					image_loaded = load(&img, arguments); // incarc imaginea
			}
			if (strcmp(command, "SELECT") == 0) {
				if (strstr(arguments, "ALL") != 0) {
					select_all(&img); // selectez intreaga imagine
				} else { // aplic functia selectcoord sau mesajul de eroare
					// daca nu se citesc exact 4 coordonate
					int x1, y1, x2, y2;
					if (sscanf(arguments, "%d%d%d%d", &x1, &y1, &x2, &y2) == 4)
						selectcoord(&img, x1, y1, x2, y2);
					else
						printf("Invalid command\n");
				}
			}
			if (strcmp(command, "HISTOGRAM") == 0)
				process_histogram_command(&img, arguments, image_loaded);
			if (strcmp(command, "EQUALIZE") == 0)
				equalize2(&img);
			if (strcmp(command, "SAVE") == 0)
				process_save_command(&img, arguments);
			if (strcmp(command, "CROP") == 0) {
				if (!image_loaded)
					printf("No image loaded\n");
				else
					crop(&img);
			}
			if (strcmp(command, "APPLY") == 0) {
				if (!image_loaded) {
					printf("No image loaded\n");
				} else {
					if (arguments)
						apply(&img, arguments);
					else
						printf("Invalid command\n");
				}
			}
			if (strcmp(command, "ROTATE") == 0) {
				int degrees;
				if (!image_loaded) {
					printf("No image loaded\n");
				} else {
					if (arguments && sscanf(arguments, "%d", &degrees) == 1)
						rotate(&img, degrees);
				}
			}
			if (strcmp(command, "EXIT") == 0) {
				if (!image_loaded)
					printf("No image loaded\n");
				free_img(&img); // eliberez imaginea din memorie
				break;
			}
		} else {
			printf("Invalid command\n");
		}
	}
	return 0;
}
