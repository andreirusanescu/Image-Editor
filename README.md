# Copyright Andrei-Marian Rusanescu 311CAb 2023-2024

The program's functionality is image editing, processing PPM or PGM
files in ASCII or binary format. It edits files that conform to the
PBM standard (with magic words P2, P3, P5, P6). The program is based
on two structures: rgb_t - a data type that stores 3 values at once
(r, g, b); and `image_t` - which contains the format in which the
loaded files are presented (magic word); the number of columns and
rows of the pixel matrix corresponding to the image; the maximum
pixel intensity value of the image; a variable with binary values
(0 or 1) that tells wheter a matrix is stored or not; and 2 matrices,
one of type int (which stores the pixel values for a black and white /
grayscale image) and the other of type rgb_t (which stores the values
for the 3 color channels of each pixel in a color image); the coordinates
of the selection on which different commands will be applied.

The following are the operations the program can perform:

1. `LOAD` Command: Loads text or binary files by applying the LOAD command,
along with the file name. If the file does not exist or could not be
opened, it releases any previously loaded image. If it opens successfully,
it checks if an image has been previously loaded and releases it in this
case. It first reads the magic word. Then, the skip function is called,
which skips any possible comments. Skip reads line by line as long as the
first character of the line is '#'. If it encounters a line that does not
start with '#', it positions the file cursor at the beginning of this line
and exits the function. Then it reads the next data, alternating with the
skip function: the number of columns and rows, the maximum pixel intensity
value, and the pixel values within the image. If the image format is P2,
an int type matrix is allocated, and for P3, an rgb_t matrix is allocated,
in the standard way. However, if the format is P5 or P6, the position where
the cursor was when the last value was read is taken, positioning the cursor
one position after the last value read. Data from binary files is read using
the unsigned char type and converted to int later. At the end of the loading,
the variable loaded in the image_t structure takes the value 1, indicating
that an image is loaded. The selection coordinates are updated, taking the
values of the image corners.

2. `Selection` Command: Applies the following commands to a selection of
pixels within the image. The selection can be the entire image (SELECT ALL)
or a submatrix whose corners are given by x1, y1, x2, y2 in the image_t
structure. From STDIN, the entire line is read, which has "SELECT" as the
first parameter and is split into multiple parameters. If the 4 parameters
after SELECT are numbers, they are converted to int data and, if valid,
are loaded into the structure.

3. `CROP` Command: The CROP command reduces the loaded image to the current
selection. It replaces the original image with a subimage of itself, using
the swap_matrix and swap_color functions.

4. `HISTOGRAM` Command: HISTOGRAM is a command that displays the histogram
of an image with up to x stars and y bins, giving the user the flexibility
to choose these values. It uses two frequency vectors, one that keeps the
frequency of each pixel within the matrix and another that sums the
frequencies of these pixels 2 by 2, 3 by 3, or x by x (depending on the
chosen number of bins). Then the first frequency vector takes the values of
the second one, and the number of stars for each bin is calculated.

5. `EQUALIZE` Command: EQUALIZE equalizes the tones of a black-and-white
(grayscale, P2 / P5) image; this effect is applied to the entire image. After
calculating these, the clamp function is called, which restricts the values
to the [0, 255] range. The values are copied to the original image.

6. `ROTATE` Command: ROTATE rotates the image at +/-270, +/-180, +/-90 degrees.
If it receives +/-360 or 0 as a parameter, the program will display that the
image has been rotated, but in fact, nothing happens "behind the scenes." The
image is rotated only to the right. If the selection is the entire image, it
will rotate regardless of its size, while if the selection is a submatrix of
the larger matrix, it will rotate only if the selection is square-shaped.

7. `APPLY` Command: APPLY is a command that can apply 4 of the image kernels
(EDGE, SHARPEN, BLUR, GAUSSIAN_BLUR) to the current selection. These image
kernels do not affect the pixels at the edges of the selection. The command can
only be applied to color images. The clamp and round functions are used to
restrict values within the [0, 255] range and ensure greater precision. One of
the 4 kernels is constructed in the "model" matrix, and the sums of the pixels,
which later become the new pixel values in the matrix, are calculated after
the filter is successfully applied.

8. `SAVE` Command: The SAVE command has 2 or 3 parameters: "SAVE", the file name
in which the current image will be saved, and optionally "ascii" if the user
wants to save the image in text format. If this parameter is not used, it will
be saved in binary mode.

9. `EXIT` Command: EXIT releases all resources used by the image editor.

The ctoi (char to int) function receives a character string as a parameter and
returns the integer it represents ("123" -> 123).

The is_num function checks if a character string is a number.
