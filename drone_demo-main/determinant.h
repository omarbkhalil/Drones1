//
// Created by bpiranda on 22/11/2019.
//

#ifndef GEOMETRY_TRIANGULATION_GL_DETERMINANT_H
#define GEOMETRY_TRIANGULATION_GL_DETERMINANT_H

class Matrix33;
class Matrix44;


/**
 * @class Matrix22
 * @brief Represents a 2x2 matrix and provides basic matrix operations.
 *
 * This class is designed to handle small matrix operations like determinant calculation
 * and extracting a 2x2 submatrix from a 3x3 matrix.
 */
class Matrix22 {
public:
    float m[2][2];///< Storage for matrix elements.

    /**
     * @brief Extracts a 2x2 submatrix from a given 3x3 matrix, ignoring specified row and column.
     * @param mat33 Reference to a Matrix33 object from which to extract.
     * @param shadowLin The row to exclude from the extraction.
     * @param shadowCol The column to exclude from the extraction.
     */
    void get2x2From3x3(const Matrix33 &mat33,int shadowLin, int shadowCol);

    /**
     * @brief Calculates and returns the determinant of this 2x2 matrix.
     * @return The determinant of the matrix.
     */
    inline float determinant() {
        return m[0][0]*m[1][1]-m[0][1]*m[1][0];
    }
};
/**
 * @class Matrix33
 * @brief Represents a 3x3 matrix and provides operations such as determinant calculation.
 *
 * This class extends matrix functionality to 3x3 matrices, including the ability to
 * extract a 3x3 matrix from a larger 4x4 matrix and compute its determinant.
 */
class Matrix33 {
public:
    float m[3][3]; ///< Storage for matrix elements.


    /**
     * @brief Extracts a 3x3 submatrix from a given 4x4 matrix, ignoring specified row and column.
     * @param mat44 Reference to a Matrix44 object from which to extract.
     * @param shadowLin The row to exclude from the extraction.
     * @param shadowCol The column to exclude from the extraction.
     */
    void get3x3From4x4(const Matrix44 &mat44,int shadowLin, int shadowCol);
    /**
     * @brief Calculates and returns the determinant of this 3x3 matrix.
     * @return The determinant of the matrix.
     */
    float determinant();
};
/**
 * @class Matrix44
 * @brief Represents a 4x4 matrix and provides a method for determinant calculation.
 *
 * The Matrix44 class is useful in applications involving 3D transformations and
 * other operations where a 4x4 matrix is required.
 */
class Matrix44 {
public:
    float m[4][4];///< Storage for matrix elements.

    /**
     * @brief Calculates and returns the determinant of this 4x4 matrix.
     * @return The determinant of the matrix.
     */
    float determinant();
};





#endif //GEOMETRY_TRIANGULATION_GL_DETERMINANT_H
