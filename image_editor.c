/* Copyright Andrei-Marian Rusanescu 311CAb 2023-2024 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// buffer len
#define commmax 255

// structure that holds values in rgb format for color matrix
typedef struct {
	int r, g, b;
} rgb_t;

// structure that holds the data for an image
typedef struct {
	char format[3]; // stores the type of an image (P2, P3 etc)
	int m, n; // columns, rows
	int maxv; // maxvalue for the intensity of the pixels
	int **matrix; // gray image
	rgb_t **color; // rgb image
	int loaded; // tells if an image is loaded
	int x1, y1, x2, y2; // selection coordinates
} image_t;

// Checks if a string is a number
int is_num(char *s)
{
	int n = strlen(s);
	int cnt = 0;
	if (s[0] == '-') {
		cnt++;
		for (int i = 1; i < n; ++i)
			if (s[i] >= '0' && s[i] <= '9')
				cnt++;
	} else {
		for (int i = 0; i < n; ++i)
			if (s[i] >= '0' && s[i] <= '9')
				cnt++;
	}
	if (cnt == n)
		return 1;
	else
		return 0;
}

// Turns a string to a number
int ctoi(char *s)
{
	int n = strlen(s);
	int nr = 0;
	if (s[0] == '-') {
		for (int i = 1; i < n; ++i)
			nr = nr * 10 + (int)(s[i] - '0');
		nr = nr * (-1);
	} else {
		for (int i = 0; i < n; ++i)
			nr = nr * 10 + (int)(s[i] - '0');
	}
	return nr;
}

// Checks if an integer is a power of two
int power2(int n)
{
	int cnt = 0;
	for (int i = 31; i >= 0; --i)
		if (n & (1 << i))
			cnt++;
	return cnt == 1;
}

// Keeps the value n in the [0, 255] interval
int clamp(int n)
{
	if (n < 0)
		return 0;
	else if (n > 255)
		return 255;
	else
		return n;
}

// Swaps two int matrices
void swap_matrix(int ***src, int ***dst)
{
	int **tmp = *src;
	*src = *dst;
	*dst = tmp;
}

// Swaps two rgb matrices
void swap_color(rgb_t ***src, rgb_t ***dst)
{
	rgb_t **tmp = *src;
	*src = *dst;
	*dst = tmp;
}

/* Skips comments in a file */
void skip(FILE *in)
{
	char s[commmax];
	while (fgets(s, commmax, in) != 0) {
		int len = strlen(s);
		s[strlen(s) - 1] = '\0';
		if (s[0] != '#') { // Not a comment
			fseek(in, -len, SEEK_CUR);
			break;
		} // Repositions by how much it read backwards
	}
}

/* Frees an int matrix */
void free_matrix(int **matrix, int n)
{
	for (int i = 0; i < n; i++)
		free(matrix[i]);
	free(matrix);
}

/* Allocates an integer matrix */
int **malloc_matrix(int rows, int columns)
{
	int **matrix = (int **)malloc(rows * sizeof(int *));
	if (!matrix) {
		fprintf(stderr, "malloc() failed\n");
		return NULL;
	}
	for (int i = 0; i < rows; i++) {
		matrix[i] = (int *)malloc(columns * sizeof(int));
		if (!matrix[i]) {
			for (int j = i - 1; j >= 0; --j)
				free(matrix[j]);
			fprintf(stderr, "malloc() failed\n");
			return NULL;
		}
	}
	return matrix;
}

/* Frees a double matrix */
void free_double(double **matrix, int n)
{
	for (int i = 0; i < n; ++i)
		free(matrix[i]);
	free(matrix);
}

/* Allocates a double matrix */
double **malloc_double(int rows, int columns)
{
	double **matrix = (double **)malloc(rows * sizeof(double *));
	if (!matrix) {
		fprintf(stderr, "malloc() failed for double\n");
		return NULL;
	}
	for (int i = 0; i < rows; ++i) {
		matrix[i] = (double *)malloc(columns * sizeof(double));
		if (!matrix[i]) {
			for (int j = i - 1; j >= 0; --j)
				free(matrix[j]);
			free(matrix);
			fprintf(stderr, "malloc() failed for double 2\n");
			return NULL;
		}
	}
	return matrix;
}

/* Frees an RGB image */
void free_color(rgb_t **color, int n)
{
	for (int i = 0; i < n; i++)
		free(color[i]);
	free(color);
}

/* Allocates memory for an RGB image*/
rgb_t **malloc_color(int rows, int columns)
{
	rgb_t **color = (rgb_t **)malloc(rows * sizeof(rgb_t *));
	if (!color) {
		fprintf(stderr, "malloc() failed\n");
		return NULL;
	}
	for (int i = 0; i < rows; i++) {
		color[i] = (rgb_t *)malloc(columns * sizeof(rgb_t));
		if (!color[i]) {
			for (int j = i - 1; j >= 0; --j)
				free(color[j]);
			fprintf(stderr, "malloc() failed\n");
			return NULL;
		}
	}
	return color;
}

/* Frees an image */
void free_photo(image_t *photo)
{
	if (photo->loaded == 1) {
		if (!strcmp(photo->format, "P2") || !strcmp(photo->format, "P5"))
			free_matrix(photo->matrix, photo->n);
		else if (!strcmp(photo->format, "P3") || !strcmp(photo->format, "P6"))
			free_color(photo->color, photo->n);
		photo->loaded = 0;
	}
}

/* Loads an image in the programme */
int loadfile(image_t *photo, char filename[])
{
	FILE *in = fopen(filename, "rb");
	if (!in) {
		printf("Failed to load %s\n", filename);
		free_photo(photo); // Frees the image
		return 1;
	} // if one was loaded before, it is freed
	if (photo->loaded == 1)
		free_photo(photo);
	fscanf(in, "%s", photo->format);
	skip(in);
	fscanf(in, "%d %d", &photo->m, &photo->n);
	skip(in);
	fscanf(in, "%d", &photo->maxv);
	skip(in);
	if (!strcmp(photo->format, "P2")) {
		photo->matrix = malloc_matrix(photo->n, photo->m);
		if (!photo->matrix) {
			free_photo(photo);
			return -1;
		}
		for (int i = 0; i < photo->n; ++i)
			for (int j = 0; j < photo->m; ++j)
				fscanf(in, "%d", &photo->matrix[i][j]);
	} else if (!strcmp(photo->format, "P3")) {
		photo->color = malloc_color(photo->n, photo->m);
		if (!photo->color) {
			free_photo(photo);
			return -1;
		}
		for (int i = 0; i < photo->n; ++i)
			for (int j = 0; j < photo->m; ++j) {
				fscanf(in, "%d", &photo->color[i][j].r);
				fscanf(in, "%d", &photo->color[i][j].g);
				fscanf(in, "%d", &photo->color[i][j].b);
			}
	} else {
		fseek(in, 1, SEEK_CUR); // moves the cursor one position
		unsigned char charc;
		if (!strcmp(photo->format, "P5")) {
			photo->matrix = malloc_matrix(photo->n, photo->m);
			if (!photo->matrix) {
				free_photo(photo);
				return -1;
			}
			for (int i = 0; i < photo->n; ++i)
				for (int j = 0; j < photo->m; ++j) {
					fread(&charc, sizeof(unsigned char), 1, in);
					photo->matrix[i][j] = (int)charc;
				}
		} else if (!strcmp(photo->format, "P6")) {
			photo->color = malloc_color(photo->n, photo->m);
			if (!photo->color) {
				free_photo(photo);
				return -1;
			}
			for (int i = 0; i < photo->n; ++i)
				for (int j = 0; j < photo->m; ++j) {
					fread(&charc, sizeof(unsigned char), 1, in);
					photo->color[i][j].r = (int)charc;
					fread(&charc, sizeof(unsigned char), 1, in);
					photo->color[i][j].g = (int)charc;
					fread(&charc, sizeof(unsigned char), 1, in);
					photo->color[i][j].b = (int)charc;
				}
		}
	}
	fclose(in);
	photo->loaded = 1;
	photo->x1 = 0;
	photo->y1 = 0;
	photo->x2 = photo->m;
	photo->y2 = photo->n;
	printf("Loaded %s\n", filename);
	return 0;
}

/* SELECT ALL and SELECT x1 y1 x2 y2 */
void select_control(image_t *s, char command[])
{
	char *p = strtok(command, "\n ");
	p = strtok(NULL, "\n ");
	if (!strcmp(p, "ALL")) {
		if (s->loaded == 1) {
			s->x1 = 0;
			s->y1 = 0;
			s->x2 = s->m;
			s->y2 = s->n;
			printf("Selected ALL\n");
		} else {
			printf("No image loaded\n");
		}
	} else { // txi and tyi are temporary values for the coords
		int tx1, tx2, ty1, ty2, n_c = 0; // checks if they are valid
		char com1[commmax], com2[commmax], com3[commmax], com4[commmax];
		while (p) {
			if (n_c == 0)
				strcpy(com1, p); // x1;
			else if (n_c == 1)
				strcpy(com2, p); // y1;
			else if (n_c == 2)
				strcpy(com3, p); // x2;
			else if (n_c == 3)
				strcpy(com4, p); // y2
			n_c++; // counts the arguments after SELECT;
			p = strtok(NULL, "\n ");
		} // if the arguments after SELECT are valid, they are being used
		if (is_num(com1) && is_num(com2) && is_num(com3) && is_num(com4)) {
			tx1 = ctoi(com1); ty1 = ctoi(com2);
			tx2 = ctoi(com3); ty2 = ctoi(com4);
			if (!s->loaded) {
				printf("No image loaded\n");
			} else if (tx1 < 0 || ty1 < 0 || tx2 < 0 || ty2 < 0 ||
			tx1 > s->m || ty1 > s->n || tx2 > s->m || ty2 > s->n ||
			tx1 == tx2 || ty1 == ty2) {
				printf("Invalid set of coordinates\n");
			} else {
				if (tx1 > tx2) {
					int aux = tx1;
					tx1 = tx2;
					tx2 = aux;
				}
				if (ty1 > ty2) {
					int aux = ty1;
					ty1 = ty2;
					ty2 = aux;
				} // loads the coordinates, they are valid
				s->x1 = tx1;
				s->y1 = ty1;
				s->x2 = tx2;
				s->y2 = ty2;
				printf("Selected %d %d %d %d\n", s->x1, s->y1, s->x2, s->y2);
			}
		} else {
			printf("Invalid command\n");
		}
	}
}

/* Crop command */
int crop_control(image_t *pht)
{
	if (pht->loaded == 0) {
		printf("No image loaded\n");
		return 0;
	} else if (!strcmp(pht->format, "P2") || !strcmp(pht->format, "P5")) {
		int **selected = malloc_matrix(pht->y2 - pht->y1, pht->x2 - pht->x1);
		if (!selected) {
			fprintf(stderr, "malloc() failed\n");
			free_photo(pht);
			return -1;
		}
		for (int i = 0; i < pht->y2 - pht->y1; ++i)
			for (int j = 0; j < pht->x2 - pht->x1; ++j)
				selected[i][j] = pht->matrix[i + pht->y1][j + pht->x1];
		/* swaps the big image with the selected one */
		swap_matrix(&selected, &pht->matrix); /* frees the old image */
		free_matrix(selected, pht->n);
	} else if (!strcmp(pht->format, "P3") || !strcmp(pht->format, "P6")) {
		rgb_t **selected = malloc_color(pht->y2 - pht->y1, pht->x2 - pht->x1);
		if (!selected) {
			fprintf(stderr, "malloc() failed\n");
			free_photo(pht);
			return -1;
		}
		for (int i = 0; i < pht->y2 - pht->y1; ++i)
			for (int j = 0; j < pht->x2 - pht->x1; ++j) {
				selected[i][j].r = pht->color[i + pht->y1][j + pht->x1].r;
				selected[i][j].g = pht->color[i + pht->y1][j + pht->x1].g;
				selected[i][j].b = pht->color[i + pht->y1][j + pht->x1].b;
			}

		swap_color(&selected, &pht->color);
		free_color(selected, pht->n);
	}
	/* Updates dimensions and selection coords */
	pht->n = pht->y2 - pht->y1;
	pht->m = pht->x2 - pht->x1;
	pht->x1 = 0;
	pht->y1 = 0;
	pht->x2 = pht->m;
	pht->y2 = pht->n;
	printf("Image cropped\n");
	return 0;
}

/* Prints the histogram of an image */
int histogram(image_t *pht, char command[])
{
	if (!pht->loaded) {
		printf("No image loaded\n");
	} else {
		char *p = strtok(command, "\n ");
		int n_c = 0;
		char com1[commmax], com2[commmax];
		while (p) {
			if (n_c == 1)
				strcpy(com1, p);
			else if (n_c == 2)
				strcpy(com2, p);
			p = strtok(NULL, "\n ");
			n_c++; // Counts arguments after Histogram
		}
		if (n_c > 3 || n_c < 2) {
			printf("Invalid command\n");
		} else if (!strcmp(pht->format, "P3") || !strcmp(pht->format, "P6")) {
			printf("Black and white image needed\n");
		} else if (is_num(com1) && is_num(com2)) {
			int x_star = ctoi(com1);
			int y_bin = ctoi(com2);
			if (y_bin < 2 || y_bin > 256 || power2(y_bin) != 1 || n_c == 2) {
				printf("Invalid set of parameters\n");
			} else {
				int *frequency, *frequency2;
				frequency = (int *)calloc(commmax + 1, sizeof(int));
				if (!frequency) {
					fprintf(stderr, "calloc() failed\n");
					free_photo(pht);
					return -1;
				}
				frequency2 = (int *)calloc(commmax + 1, sizeof(int));
				if (!frequency2) {
					fprintf(stderr, "calloc() failed\n");
					free_photo(pht);
					free(frequency);
					return -1;
				} // Computes pixels' frequency
				for (int i = 0; i < pht->n; ++i)
					for (int j = 0; j < pht->m; ++j)
						frequency[pht->matrix[i][j]]++;

				// computes how the values will group for y bins
				int groups = 256 / y_bin, cnt = 0, maxim = 0;
				// computes frequency group by group
				for (int i = 0; i < 256; i += groups) {
					for (int j = i; j < i + groups; ++j)
						frequency2[cnt] += frequency[j];
					if (frequency2[cnt] > maxim)
						maxim = frequency2[cnt];
					cnt++; // basically, adds all the frequencies in a group
				}
				// assigns the new frequency
				for (int i = 0; i < y_bin; ++i)
					frequency[i] = frequency2[i];
				free(frequency2);
				for (int i = 0; i < y_bin; ++i) {
					int stars = (frequency[i] * x_star) / maxim;
					printf("%d\t|\t", stars);
					for (int j = 0; j < stars; ++j)
						printf("*");
					printf("\n");
				}
				free(frequency);
			}
		} else {
			printf("Invalid command\n");
		}
	}
	return 0;
}

/* Applies equalize effect to an image */
int equalize(image_t *photo, char command[])
{
	char *p = strtok(command, "\n ");
	int n_c = 0;
	while (p) {
		n_c++;
		p = strtok(NULL, "\n ");
	}
	if (!photo->loaded) {
		printf("No image loaded\n");
	} else if (n_c > 1) {
		printf("Invalid command\n");
	} else if (!strcmp(photo->format, "P3") || !strcmp(photo->format, "P6")) {
		printf("Black and white image needed\n");
	} else {
		// frequency array
		int *h = (int *)calloc(photo->maxv + 1, sizeof(int));
		if (!h) {
			fprintf(stderr, "calloc() failed\n");
			free(photo);
			return -1;
		}
		for (int i = 0; i < photo->n; ++i)
			for (int j = 0; j < photo->m; ++j)
				h[photo->matrix[i][j]]++;

		double sum = 0;
		for (int i = 0; i < photo->n; ++i) {
			for (int j = 0; j < photo->m; ++j) {
				sum = 0;
				for (int k = 0; k <= photo->matrix[i][j]; ++k)
					sum += (double)h[k];

				photo->matrix[i][j] = clamp(255. / (photo->n * photo->m) * sum);
			}
		}
		printf("Equalize done\n");
		free(h);
	}
	return 0;
}

/* Rotates a selection of an image */
int rotate(image_t *photo)
{
	int **rotatie = malloc_matrix(photo->y2 - photo->y1, photo->y2 - photo->y1);
	int **select = malloc_matrix(photo->y2 - photo->y1, photo->y2 - photo->y1);
	if (!rotatie) {
		fprintf(stderr, "malloc() failed\n");
		free_photo(photo);
		return -1;
	}
	if (!select) {
		fprintf(stderr, "malloc() failed\n");
		free_photo(photo);
		free_matrix(rotatie, photo->y2 - photo->y1);
		return -1;
	}
	int n = photo->y2 - photo->y1, m = photo->x2 - photo->x1;
	for (int i = photo->y1; i < photo->y2; ++i)
		for (int j = photo->x1; j < photo->x2; ++j) // loads up the selection
			select[i - photo->y1][j - photo->x1] = photo->matrix[i][j];

	for (int j = 0; j < m; ++j)
		for (int i = n - 1; i >= 0; --i)
			rotatie[j][n - i - 1] = select[i][j]; // formula for rotation

	for (int i = photo->y1; i < photo->y2; ++i) // loads up the rotated matrix
		for (int j = photo->x1; j < photo->x2; ++j)
			photo->matrix[i][j] = rotatie[i - photo->y1][j - photo->x1];

	free_matrix(rotatie, photo->y2 - photo->y1);
	free_matrix(select, photo->y2 - photo->y1);
	return 0;
}

/* Rotates a selection of a colored image */
int rotatecolor(image_t *photo)
{
	rgb_t **rotate = malloc_color(photo->y2 - photo->y1, photo->y2 - photo->y1);
	rgb_t **select = malloc_color(photo->y2 - photo->y1, photo->y2 - photo->y1);
	if (!rotate) {
		fprintf(stderr, "malloc() failed\n");
		free_photo(photo);
		return -1;
	}
	if (!select) {
		fprintf(stderr, "malloc() failed\n");
		free_photo(photo);
		free_color(rotate, photo->y2 - photo->y1);
		return -1;
	}
	int n = photo->y2 - photo->y1, m = photo->x2 - photo->x1;
	for (int i = photo->y1; i < photo->y2; ++i) {
		for (int j = photo->x1; j < photo->x2; ++j) {
			select[i - photo->y1][j - photo->x1].r = photo->color[i][j].r;
			select[i - photo->y1][j - photo->x1].g = photo->color[i][j].g;
			select[i - photo->y1][j - photo->x1].b = photo->color[i][j].b;
		}
	}
	for (int j = 0; j < m; ++j) {
		for (int i = n - 1; i >= 0; --i) {
			rotate[j][n - i - 1].r = select[i][j].r;
			rotate[j][n - i - 1].g = select[i][j].g;
			rotate[j][n - i - 1].b = select[i][j].b;
		}
	}
	for (int i = photo->y1; i < photo->y2; ++i) {
		for (int j = photo->x1; j < photo->x2; ++j) {
			photo->color[i][j].r = rotate[i - photo->y1][j - photo->x1].r;
			photo->color[i][j].g = rotate[i - photo->y1][j - photo->x1].g;
			photo->color[i][j].b = rotate[i - photo->y1][j - photo->x1].b;
		}
	}

	free_color(rotate, photo->y2 - photo->y1);
	free_color(select, photo->y2 - photo->y1);
	return 0;
}

/* Rotates the whole image */
int rotatefull(image_t *photo, char command[], int cnt)
{
	if (!strcmp(photo->format, "P2") || !strcmp(photo->format, "P5")) {
		for (int k = 0; k < cnt; ++k) { // gray
			int **rotate = malloc_matrix(photo->m, photo->n);
			if (!rotate) {
				fprintf(stderr, "malloc() failed @rotate\n");
				free_photo(photo);
				return -1;
			}
			for (int i = 0; i < photo->n; ++i)
				for (int j = 0; j < photo->m; ++j)
					rotate[j][i] = photo->matrix[photo->n - i - 1][j];

			swap_matrix(&rotate, &photo->matrix);
			free_matrix(rotate, photo->n);
			int tmp = photo->n;
			photo->n = photo->m;
			photo->m = tmp;
			tmp = photo->y2;
			photo->y2 = photo->x2;
			photo->x2 = tmp;
		}
	} else {
		for (int k = 0; k < cnt; ++k) { // color
			rgb_t **rotate = malloc_color(photo->m, photo->n);
			if (!rotate) {
				fprintf(stderr, "malloc() failed @rotate\n");
				free_photo(photo);
				return -1;
			}
			for (int i = 0; i < photo->n; ++i) {
				for (int j = 0; j < photo->m; ++j) {
					rotate[j][i].r = photo->color[photo->n - i - 1][j].r;
					rotate[j][i].g = photo->color[photo->n - i - 1][j].g;
					rotate[j][i].b = photo->color[photo->n - i - 1][j].b;
				}
			}
			swap_color(&rotate, &photo->color);
			free_color(rotate, photo->n);
			int tmp = photo->n;
			photo->n = photo->m;
			photo->m = tmp;
			tmp = photo->y2;
			photo->y2 = photo->x2;
			photo->x2 = tmp;
		}
	}
	printf("Rotated %s\n", command);
	return 0;
}

/* Handles rotations for selections of an image */
int rotate_angle(image_t *photo, char command[], int cnt)
{
	// gray rotation:
	if (!strcmp(photo->format, "P2") || !strcmp(photo->format, "P5")) {
		for (int i = 0; i < cnt; ++i) {
			int okrot = rotate(photo);
			if (okrot == -1) {
				free_photo(photo);
				return -1;
			}
		}
		// RGB rotation:
	} else {
		for (int i = 0; i < cnt; ++i) {
			int okrot = rotatecolor(photo);
			if (okrot == -1) {
				free_photo(photo);
				return -1;
			}
		}
	}
	printf("Rotated %s\n", command);
	return 0;
}

/* Rotate driver */
int rotatecommand(image_t *photo, char command[])
{
	int angle;
	if (!photo->loaded) {
		printf("No image loaded\n");
	} else if (!strcmp(command, "0") || !strcmp(command, "360") ||
	!strcmp(command, "-360")) {
		printf("Rotated %s\n", command);
	} else if (photo->x1 == 0 && photo->y1 == 0 &&
	photo->x2 == photo->m && photo->y2 == photo->n) {
		if (!strcmp(command, "90") || !strcmp(command, "-270")) {
			angle = rotatefull(photo, command, 1);
			if (angle == -1)
				return -1;
		} else if (!strcmp(command, "180") || !strcmp(command, "-180")) {
			angle = rotatefull(photo, command, 2);
			if (angle == -1)
				return -1;
		} else if (!strcmp(command, "270") || !strcmp(command, "-90")) {
			angle = rotatefull(photo, command, 3);
			if (angle == -1)
				return -1;
		} else {
			printf("Unsupported rotation angle\n");
		}
	} else if (photo->y2 - photo->y1 != photo->x2 - photo->x1) {
		printf("The selection must be square\n");
	} else {
		if (!strcmp(command, "90") || !strcmp(command, "-270")) {
			angle = rotate_angle(photo, command, 1);
			if (angle == -1)
				return -1;
		} else if (!strcmp(command, "180") || !strcmp(command, "-180")) {
			angle = rotate_angle(photo, command, 2);
			if (angle == -1)
				return -1;
		} else if (!strcmp(command, "270") || !strcmp(command, "-90")) {
			angle = rotate_angle(photo, command, 3);
			if (angle == -1)
				return -1;
		} else {
			printf("Unsupported rotation angle\n");
		}
	}
	return 0;
}

/* Applies a filter to an image */
int apply_it(image_t *s, double **model)
{
	rgb_t **copy = malloc_color(s->n, s->m);
	if (!copy) {
		fprintf(stderr, "malloc() failed at apply it\n");
		return -1;
	} // Operations are performed on an auxiliary matrix
	for (int i = 0; i < s->n; ++i) {
		for (int j = 0; j < s->m; ++j) {
			copy[i][j].r = s->color[i][j].r;
			copy[i][j].g = s->color[i][j].g;
			copy[i][j].b = s->color[i][j].b;
		}
	}
	for (int i = s->y1; i < s->y2; ++i) {
		for (int j = s->x1; j < s->x2; ++j) {
			// edges are not taken into account
			if (i > 0 && i < s->n - 1 && j > 0 && j < s->m - 1) {
				double sred = 0, sgreen = 0, sblue = 0;
				for (int k = i - 1, p = 0; k < i + 2; ++k, ++p) {
					for (int l = j - 1, q = 0; l < j + 2; ++l, ++q) {
						sred += (double)s->color[k][l].r * model[p][q];
						sgreen += (double)s->color[k][l].g * model[p][q];
						sblue += (double)s->color[k][l].b * model[p][q];
					}
				}
				if (model[1][1] == 1) { // blur
					copy[i][j].r = clamp(round((1. / 9) * sred));
					copy[i][j].g = clamp(round((1. / 9) * sgreen));
					copy[i][j].b = clamp(round((1. / 9) * sblue));
				} else if (model[1][1] == 4) { // gauss
					copy[i][j].r = clamp(round((1. / 16) * sred));
					copy[i][j].g = clamp(round((1. / 16) * sgreen));
					copy[i][j].b = clamp(round((1. / 16) * sblue));
				} else if (model[1][1] == 8 || model[1][1] == 5) {
					copy[i][j].r = clamp(round(sred));
					copy[i][j].g = clamp(round(sgreen));
					copy[i][j].b = clamp(round(sblue));
				} // sharpen && edge
			}
		}
	}

	for (int i = 0; i < s->n; ++i) {
		for (int j = 0; j < s->m; ++j) {
			s->color[i][j].r = copy[i][j].r;
			s->color[i][j].g = copy[i][j].g;
			s->color[i][j].b = copy[i][j].b;
		}
	}
	free_color(copy, s->n);
	return 0;
}

/* Builds EDGE matrix */
void build_edge(double ***edge)
{
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (i == 1 && j == 1)
				(*edge)[i][j] = 8;
			else
				(*edge)[i][j] = -1;
		}
	}
}

/* Builds SHARPEN matrix */
void build_sharpen(double ***sharpen)
{
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (i == 1 && j == 1) {
				(*sharpen)[i][j] = 5;
			} else if ((i == 0 && j == 0) || (i == 0 && j == 2) ||
			(i == 2 && j == 0) || (i == 2 && j == 2)) {
				(*sharpen)[i][j] = 0;
			} else {
				(*sharpen)[i][j] = -1;
			}
		}
	}
}

/* Builds GAUSSIAN_BLUR matrix*/
void build_gauss(double ***gauss)
{
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (i == 1 && j == 1) {
				(*gauss)[i][j] = 4;
			} else if ((i == 0 && j == 0) || (i == 0 && j == 2) ||
			(i == 2 && j == 0) || (i == 2 && j == 2)) {
				(*gauss)[i][j] = 1;
			} else {
				(*gauss)[i][j] = 2;
			}
		}
	}
}

/* Handles the filters applied to an image */
int apply_control(image_t *photo, char command[])
{
	if (!strcmp(command, "EDGE")) {
		double **edge = malloc_double(3, 3);
		if (!edge) {
			fprintf(stderr, "malloc() failed for EDGE\n");
			free_photo(photo);
			return -1;
		}
		build_edge(&edge);
		int a = apply_it(photo, edge);
		free_double(edge, 3);
		if (a == -1) {
			free_photo(photo);
			return -1;
		}
		return 0;
	} else if (!strcmp(command, "SHARPEN")) {
		double **sharpen = malloc_double(3, 3);
		if (!sharpen) {
			fprintf(stderr, "malloc() failed for SHARPEN\n");
			free_photo(photo);
			return -1;
		}
		build_sharpen(&sharpen);
		int a = apply_it(photo, sharpen);
		free_double(sharpen, 3);
		if (a == -1) {
			free_photo(photo);
			return -1;
		}
		return 0;
	} else if (!strcmp(command, "BLUR")) {
		double **blur = malloc_double(3, 3);
		if (!blur) {
			fprintf(stderr, "malloc() failed for BLUR\n");
			free_photo(photo);
			return -1;
		}
		for (int i = 0; i < 3; ++i)
			for (int j = 0; j < 3; ++j)
				blur[i][j] = 1;
		int a = apply_it(photo, blur);
		free_double(blur, 3);
		if (a == -1) {
			free_photo(photo);
			return -1;
		}
		return 0;
	} else if (!strcmp(command, "GAUSSIAN_BLUR")) {
		double **gauss = malloc_double(3, 3);
		if (!gauss) {
			fprintf(stderr, "malloc() faiiled for GAUSS\n");
			free_photo(photo);
			return -1;
		}
		build_gauss(&gauss);
		int a = apply_it(photo, gauss);
		free_double(gauss, 3);
		if (a == -1) {
			free_photo(photo);
			return -1;
		}
		return 0;
	} else {
		return 1;
	}
}

/* Handles apply command */
int apply_centre(image_t *photo, char comm[])
{
	char *p = strtok(comm, "\n "), command[commmax];
	int n_c = 0;
	while (p) {
		if (n_c == 1)
			strcpy(command, p);
		n_c++;
		p = strtok(NULL, "\n ");
	}
	if (!photo->loaded) {
		printf("No image loaded\n");
	} else if (n_c != 2) {
		printf("Invalid command\n");
	} else if (!strcmp(photo->format, "P2") || !strcmp(photo->format, "P5")) {
		printf("Easy, Charlie Chaplin\n");
	} else {
		int cod = apply_control(photo, command);
		if (!cod)
			printf("APPLY %s done\n", command);
		else if (cod == -1)
			return -1;
		else if (cod == 1)
			printf("APPLY parameter invalid\n");
	}
	return 0;
}

/* Saves file in text or binary format */
void save_type(image_t *pht, char filename[], int text)
{
	FILE *out = fopen(filename, "wb");
	if (!out) {
		printf("Couldn't open file %s\n", filename);
		return;
	}
	if (text == 1) {
		if (!strcmp(pht->format, "P2") || !strcmp(pht->format, "P5")) {
			fprintf(out, "%s\n", "P2");
			fprintf(out, "%d %d\n%d\n", pht->m, pht->n, pht->maxv);
			for (int i = 0; i < pht->n; ++i) {
				for (int j = 0; j < pht->m; ++j)
					fprintf(out, "%d ", pht->matrix[i][j]);
				fprintf(out, "\n");
			}
		} else if (!strcmp(pht->format, "P3") || !strcmp(pht->format, "P6")) {
			fprintf(out, "%s\n", "P3");
			fprintf(out, "%d %d\n%d\n", pht->m, pht->n, pht->maxv);
			for (int i = 0; i < pht->n; ++i) {
				for (int j = 0; j < pht->m; ++j) {
					fprintf(out, "%d ", pht->color[i][j].r);
					fprintf(out, "%d ", pht->color[i][j].g);
					fprintf(out, "%d ", pht->color[i][j].b);
				}
				fprintf(out, "\n");
			}
		}
	} else {
		if (!strcmp(pht->format, "P2") || !strcmp(pht->format, "P5")) {
			fprintf(out, "%s\n", "P5");
			fprintf(out, "%d %d\n%d\n", pht->m, pht->n, pht->maxv);
			unsigned char charc;
			for (int i = 0; i < pht->n; ++i)
				for (int j = 0; j < pht->m; ++j) {
					charc = (unsigned char)pht->matrix[i][j];
					fwrite(&charc, sizeof(unsigned char), 1, out);
				} // Saves pixels as unsigned char values
		} else if (!strcmp(pht->format, "P3") || !strcmp(pht->format, "P6")) {
			fprintf(out, "%s\n", "P6");
			fprintf(out, "%d %d\n%d\n", pht->m, pht->n, pht->maxv);
			unsigned char charc;
			for (int i = 0; i < pht->n; ++i)
				for (int j = 0; j < pht->m; ++j) {
					charc = (unsigned char)pht->color[i][j].r;
					fwrite(&charc, sizeof(unsigned char), 1, out);
					charc = (unsigned char)pht->color[i][j].g;
					fwrite(&charc, sizeof(unsigned char), 1, out);
					charc = (unsigned char)pht->color[i][j].b;
					fwrite(&charc, sizeof(unsigned char), 1, out);
				}
		}
	}
	fclose(out);
}

/* Save command*/
void save(image_t *photo, char command[])
{
	char filename[commmax];
	char *p = strtok(command, "\n ");
	int n_c = 0;
	while (p) {
		if (n_c == 1)
			strcpy(filename, p);
		else if (n_c == 2)
			break; // p is kept
		p = strtok(NULL, "\n ");
		n_c++;
	}
	int text = 0;
	if (n_c == 3 && !strcmp(p, "ascii"))
		text = 1; // saves in text format
	if (photo->loaded == 0) {
		printf("No image loaded\n");
	} else {
		save_type(photo, filename, text);
		printf("Saved %s\n", filename);
	}
}

/* Exit command */
void exitC(image_t *photo)
{
	if (photo->loaded == 0)
		printf("No image loaded\n");
	else
		free_photo(photo);
}

int main(void)
{
	image_t photo;
	char longcomm[commmax];
	photo.loaded = 0; // no image loaded initially
	int ok = 1;
	while (ok) {
		char copy[commmax]; // holds the line
		int n_c = 0; // counts line args
		char command[commmax], command2[commmax];
		fgets(longcomm, commmax, stdin);
		strcpy(copy, longcomm);
		char *p = strtok(longcomm, "\n ");
		while (p) {
			if (n_c == 0) // first command
				strcpy(command, p);
			else if (n_c == 1) // second
				strcpy(command2, p);
			n_c++;
			p = strtok(NULL, "\n ");
		}

		if (!strcmp(command, "LOAD") && n_c == 2) {
			int loadC = loadfile(&photo, command2);
			if (loadC == -1) {
				fprintf(stderr, "malloc() failed\n");
				return -1;
			}
		} else if (!strcmp(command, "SELECT")) {
			select_control(&photo, copy);
		} else if (!strcmp(command, "HISTOGRAM")) {
			int histo = histogram(&photo, copy);
			if (histo == -1)
				return -1;
		} else if (!strcmp(command, "EQUALIZE")) {
			int eq = equalize(&photo, copy);
			if (eq == -1)
				return -1;
		} else if (!strcmp(command, "ROTATE")) {
			int rot = rotatecommand(&photo, command2);
			if (rot == -1)
				return -1;
		} else if (!strcmp(command, "CROP")) {
			int cropC = crop_control(&photo);
			if (cropC == -1)
				return -1;
		} else if (!strcmp(command, "APPLY")) {
			int app = apply_centre(&photo, copy);
			if (app == -1)
				return -1;
		} else if (!strcmp(command, "SAVE")) {
			save(&photo, copy);
		} else if (!strcmp(command, "EXIT")) {
			exitC(&photo);
			ok = 0;
		} else {
			printf("Invalid command\n");
		}
	}
	return 0;
}
