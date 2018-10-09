#include <mpi.h>
#include <iostream>
#include <math.h>
#include <time.h>
#include <limits.h>

using namespace std;

void printMatrix(int* matrix, int row, int col){
	if (matrix){
		for (int i = 0; i < row; i++){
			for (int j = 0; j < col; j++){
				cout << matrix[i*col + j] << " ";
			}
			cout << endl;
		}
		cout << endl;
	}
}

int* createMatrix(int rows, int columns){
	int *matrix;
	matrix = new int[rows*columns];
	return matrix;
}

int* createVector(int rows){
	int *result;
	result = new int[rows];
	return result;
}

void Rand(int *matrix, int rows, int columns){
	for (int i = 0; i < rows; i++){
		for (int j = 0; j < columns; j++){
			matrix[i * columns + j] = rand()%100;
		}
	}
}

int main(int argc, char** argv){
	int rows;
	int columns;
	int *matrix, *tmp = NULL, *result = NULL;
	int ProcSize, ProcRank;
	MPI_Status Status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &ProcSize);
	MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
	if (ProcRank == 0){
		cout << "Enter the number of rows=";
		cin >> rows;
		cout << "Enter the number of columns=";
		cin >> columns;

		matrix = createMatrix(rows, columns);
		tmp = createVector(rows);
		result = new int[rows];
		for (int i = 0; i < rows; i++)
			result[i] = INT_MIN;


		Rand(matrix, rows, columns);

		if (rows < 10 && columns < 10)
			printMatrix(matrix, rows, columns);

		for (int i = 1; i < ProcSize; i++){
			MPI_Send(&columns, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&rows, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(matrix, columns*rows, MPI_INT, i, 0, MPI_COMM_WORLD);
		}

		for (int i = 1; i < ProcSize; i++){
			MPI_Recv(tmp, rows, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
			for (int j = 0; j < rows; j++){
				if (tmp[j] > result[j])
					result[j] = tmp[j];
			}
		}

		cout << "Result = ";
		for (int i = 0; i < rows; i++)
			cout << result[i] << " ";
		cout << endl;

		delete matrix;
		delete tmp;
		delete result;
	}
	else{
		MPI_Recv(&columns, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
		MPI_Recv(&rows, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);

		matrix = createMatrix(rows, columns);
		tmp = createVector(rows);
		MPI_Recv(matrix, columns*rows, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
		for (int i = 0; i < rows; i++){
			tmp[i] = INT_MIN;
			for (int j = (ProcRank - 1); j < columns; j += (ProcSize - 1))
			if (matrix[i * columns + j] > tmp[i])
				tmp[i] = matrix[i * columns + j];
		}
		MPI_Send(tmp, rows, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Finalize();
	return 0;
}
