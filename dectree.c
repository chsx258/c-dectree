/*
 * This code is provided solely for the personal and private use of students
 * taking the CSC209H course at the University of Toronto. Copying for purposes
 * other than this use is expressly prohibited. All forms of distribution of
 * this code, including but not limited to public repositories on GitHub,
 * GitLab, Bitbucket, or any other online platform, whether as given or with
 * any changes, are expressly prohibited.
 *
 * Authors: Mustafa Quraish, Bianca Schroeder, Karen Reid
 *
 * All of the files in this directory and all subdirectories are:
 * Copyright (c) 2021 Karen Reid
 */

#include "dectree.h"

/**
 * Load the binary file, filename into a Dataset and return a pointer to 
 * the Dataset. The binary file format is as follows:
 *
 *     -   4 bytes : `N`: Number of images / labels in the file
 *     -   1 byte  : Image 1 label
 *     - NUM_PIXELS bytes : Image 1 data (WIDTHxWIDTH)
 *          ...
 *     -   1 byte  : Image N label
 *     - NUM_PIXELS bytes : Image N data (WIDTHxWIDTH)
 *
 * You can set the `sx` and `sy` values for all the images to WIDTH. 
 * Use the NUM_PIXELS and WIDTH constants defined in dectree.h
 */
Dataset *load_dataset(const char *filename) {
    // TODO: Allocate data, read image data / labels, return
    FILE *fp;
    fp = fopen(filename,"r+");
    Dataset *data = malloc(sizeof(Dataset));
    data->num_items = 0;
    fread(&data->num_items,4,1,fp);
    data->images = malloc(sizeof(Image)*(data->num_items));
    data->labels = malloc(sizeof(unsigned char)*(data->num_items));

    for(int i =0; i<data->num_items;i++){
        fread(&data->labels[i],1,1,fp);
        Image *image = &data->images[i];
        image->sx = WIDTH;
        image->sy = WIDTH;
        image->data = malloc(sizeof(unsigned char)*NUM_PIXELS);
        fread(image->data,1,NUM_PIXELS,fp);
        //printf("%d\n",data->labels[i]);
        //printf("value is %d\n",image->data[200]);
    }
    return data;
    return NULL;
}

/**
 * Compute and return the Gini impurity of M images at a given pixel
 * The M images to analyze are identified by the indices array. The M
 * elements of the indices array are indices into data.
 * This is the objective function that you will use to identify the best 
 * pixel on which to split the dataset when building the decision tree.
 *
 * Note that the gini_impurity implemented here can evaluate to NAN 
 * (Not A Number) and will return that value. Your implementation of the 
 * decision trees should ensure that a pixel whose gini_impurity evaluates 
 * to NAN is not used to split the data.  (see find_best_split)
 * 
 * DO NOT CHANGE THIS FUNCTION; It is already implemented for you.
 */
double gini_impurity(Dataset *data, int M, int *indices, int pixel) {
    int a_freq[10] = {0}, a_count = 0;
    int b_freq[10] = {0}, b_count = 0;

    for (int i = 0; i < M; i++) {
        int img_idx = indices[i];
        // The pixels are always either 0 or 255, but using < 128 for generality.
        if (data->images[img_idx].data[pixel] < 128) {
            a_freq[data->labels[img_idx]]++;
            a_count++;
        } else {
            b_freq[data->labels[img_idx]]++;
            b_count++;
        }
    }

    double a_gini = 0, b_gini = 0;
    for (int i = 0; i < 10; i++) {
        double a_i = ((double)a_freq[i]) / ((double)a_count);
        double b_i = ((double)b_freq[i]) / ((double)b_count);
        a_gini += a_i * (1 - a_i);
        b_gini += b_i * (1 - b_i);
    }

    // Weighted average of gini impurity of children
    return (a_gini * a_count + b_gini * b_count) / M;
}

/**
 * Given a subset of M images and the array of their corresponding indices, 
 * find and use the last two parameters (label and freq) to store the most
 * frequent label in the set and its frequency.
 *
 * - The most frequent label (between 0 and 9) will be stored in `*label`
 * - The frequency of this label within the subset will be stored in `*freq`
 * 
 * If multiple labels have the same maximal frequency, return the smallest one.
 */
void get_most_frequent(Dataset *data, int M, int *indices, int *label, int *freq) {
    // TODO: Set the correct values and return
    *label = 0;
    *freq = 0;
    int labels[10] = {0,0,0,0,0,0,0,0,0,0};
    for (int i=0; i< M; i++){
        int img_idx = indices[i];
        labels[data->labels[img_idx]] += 1;
    }
    *freq = -1;
    for (int i = 0; i<10; i++){
        if (labels[i]>*freq){
            *freq = labels[i];
            *label = i;
            }
    }
    //printf("most freq is %d with %d \n",*label,*freq);
    return;
}

/**
 * Given a subset of M images as defined by their indices, find and return
 * the best pixel to split the data. The best pixel is the one which
 * has the minimum Gini impurity as computed by `gini_impurity()` and 
 * is not NAN. (See handout for more information)
 * 
 * The return value will be a number between 0-783 (inclusive), representing
 *  the pixel the M images should be split based on.
 * 
 * If multiple pixels have the same minimal Gini impurity, return the smallest.
 */
int find_best_split(Dataset *data, int M, int *indices) {
    // TODO: Return the correct pixel
    double min = 0x3f3f3f;
    //printf("min is %fl \n",min);
    int ans = 0;

    for (int i=1; i<NUM_PIXELS; i++){
        double temp = gini_impurity(data,M,indices,i);
//        printf("temp is %fl \n",temp);

        if (temp < min){
            min = temp;
            ans = 1+i;
            ans -=1;
            //printf("ans is %d \n",ans);
        }
    }
    //printf("finded %d \n",ans);
    return ans;
    return 0;
}

/**
 * Create the Decision tree. In each recursive call, we consider the subset of the
 * dataset that correspond to the new node. To represent the subset, we pass 
 * an array of indices of these images in the subset of the dataset, along with 
 * its length M. Be careful to allocate this indices array for any recursive 
 * calls made, and free it when you no longer need the array. In this function,
 * you need to:
 *
 *    - Compute ratio of most frequent image in indices, do not split if the
 *      ration is greater than THRESHOLD_RATIO
 *    - Find the best pixel to split on using `find_best_split`
 *    - Split the data based on whether pixel is less than 128, allocate 
 *      arrays of indices of training images and populate them with the 
 *      subset of indices from M that correspond to which side of the split
 *      they are on
 *    - Allocate a new node, set the correct values and return
 *       - If it is a leaf node set `classification`, and both children = NULL.
 *       - Otherwise, set `pixel` and `left`/`right` nodes 
 *         (using build_subtree recursively). 
 */
DTNode *build_subtree(Dataset *data, int M, int *indices) {
    // TODO: Construct and return the tree
    int *label = malloc(sizeof(unsigned char));
    int *freq = malloc(sizeof(int));
    get_most_frequent(data,M,indices,label,freq);
    double ratio = (float)*freq/M;
    free(freq);
    DTNode *node = (DTNode*)malloc(sizeof(DTNode));
    node->pixel = find_best_split(data,M,indices);
    if (ratio >= THRESHOLD_RATIO){
        node->classification = *label;
        node->left=NULL;
        node->right=NULL;
        free(indices);
    }
    else{
        int *ind1 = malloc(sizeof(int)*M);
        int *ind2 = malloc(sizeof(int)*M);
        int a = 0;
        int b = 0;
        node->classification = -1;
        for (int i=0;i<M;i++){
            int img_idx = indices[i];
            int pixel = node->pixel;
            if (data->images[img_idx].data[pixel]<128){
                ind1[a] = indices[i];
                a++;
            }
            else{
                ind2[b] = indices[i];
                b++;
            }
        }
        node->left = build_subtree(data,a,ind1);
        node->right = build_subtree(data,b,ind2);
        free(indices);
    }
    //printf("made a level of tree \n");
    free(label);
    return node;
    return NULL;
}

/**
 * This is the function exposed to the user. All you should do here is set
 * up the `indices` array correctly for the entire dataset and call 
 * `build_subtree()` with the correct parameters.
 */
DTNode *build_dec_tree(Dataset *data) {
    // TODO: Set up `indices` array, call `build_subtree` and return the tree.
    // HINT: Make sure you free any data that is not needed anymore
    int *indices = malloc(sizeof(int)*data->num_items);
    for (int i =0;i<data->num_items;i++){
        indices[i] = i;
    }
    return build_subtree(data,data->num_items,indices);
    return NULL;
}

/**
 * Given a decision tree and an image to classify, return the predicted label.
 */
int dec_tree_classify(DTNode *root, Image *img) {
    // TODO: Return the correct label
    int pix = root->pixel;
    if (root->classification != -1){return root->classification;}
    if (img->data[pix]<128){
        return dec_tree_classify(root->left,img);
    }
    return dec_tree_classify(root->right,img);
    return -1;
}

/**
 * This function frees the Decision tree.
 */
void free_dec_tree(DTNode *node) {
    // TODO: Free the decision tree
    if (node->left){free_dec_tree(node->left);}
    if (node->right){free_dec_tree(node->right);}
    free(node);
    return;
}

/**
 * Free all the allocated memory for the dataset
 */
void free_dataset(Dataset *data) {
    // TODO: Free dataset (Same as A1)
    for (int i=data->num_items-1;i>-1;i--){
        Image *point1 = &data->images[i];
        free(point1->data);
    }
    free(data->images);
    free(data->labels);
    free(data);
    return;
}
