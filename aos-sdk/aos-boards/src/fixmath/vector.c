#include "fix16.h"

/*
 * Function fix16_vector_dot_product
 * Action: calculate the dot product between two vectors
 * Input:
 *   v1: vector 1
 *   v2: vector 2
 * output: None
 * return: the dot product
 * Comments: None
 */
fix16_t fix16_vector_dot_product(fix16_vector_t* v1, fix16_vector_t* v2)
{
	fix16_t temp;
	fix16_t sum;

	sum = fix16_mul(v1->x, v2->x);
	temp = fix16_mul(v1->y, v2->y);
	sum = fix16_add(sum, temp);
	temp = fix16_mul(v1->z, v2->z);
	sum = fix16_add(sum, temp);
	return sum;
}

/*
 * Function fix16_vector_magnitude
 * Action: calculate the magnitude of a vector
 * Input:
 *   v: vector
 * output: None
 * return: the vector magnitude
 * Comments: None
 */
fix16_t fix16_vector_magnitude(fix16_vector_t* v)
{
	fix16_t result;

	result = fix16_vector_squared_magnitude(v);

	return fix16_sqrt(result);
}


/*
 * Function fix16_vector_squared_magnitude
 * Action: calculate the squared magnitude of a vector
 * Input:
 *   v: vector
 * output: None
 * return: the vector squared magnitude
 * Comments: None
 */
fix16_t fix16_vector_squared_magnitude(fix16_vector_t* v)
{
	fix16_t temp;
	fix16_t sum;

	sum = fix16_mul(v->x, v->x);
	temp = fix16_mul(v->y, v->y);
	sum = fix16_add(sum, temp);
	temp = fix16_mul(v->z, v->z);
	return fix16_add(sum, temp);
}


/*
 * Function fix16_vectors_angle
 * Action: return the angle between 2 vectors
 * Input:
 *  v1: vector 1
 *  v2: vector 2
 * output: None
 * return: The angle
 * Comments:
 * It is calculated from the dot product of the two vectors as:
 *
 *  teta = arccos(dot(v1,v2)/(||v1|| *|||v2||)
 *
 */
fix16_t fix16_vectors_angle(fix16_vector_t* v1, fix16_vector_t* v2)
{
	fix16_t dot;
	fix16_t temp;

	dot = fix16_vector_dot_product(v1, v2);
	temp = fix16_vector_magnitude(v1);
	temp = fix16_mul(temp,fix16_vector_magnitude(v2));

	// This case can happen when an angle is very close to 0 degrees
	if(fix16_abs(dot) >= temp) {
		if (dot > 0) {
			return 0;
		} else {
			return fix16_pi;
		}
	}

	temp = fix16_div(dot, temp);
	return fix16_acos(temp);
}


