#include <iostream>
#include <mpi.h>
#include <string>
using namespace std;


void printMatrix(int *data, int size) {
	if (size < 15 ) {
		for (int i = 0; i < size; i++) {
			for (int j = 0; j < size; j++) {
				cout << data[i * size + j] << " ";
			}
			cout << endl;
		}
	}
}

int* createMatrix(int size) {
	int *matrix;
	matrix = new int[size*size];
	return matrix;
}

void fullMatrix(int *matrix, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			matrix[i*size + j] = rand() % 100;
		}
	}
}
int maxSearch(int a, int b) {
	if (a >= b)
		return a;
	else return b;
}
int main(int argc, char **argv) {
	int rank, size;
	MPI_Status Status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	int Size = stoi(string(argv[1]));	
	int *matrix = nullptr;

	if (rank == 0) {
		matrix = new int[Size * Size];
		fullMatrix(matrix, Size);
		printMatrix(matrix, Size);
	}

	int partSize = Size / size;
	int *vec = new int[Size * partSize];

	if (size > Size) {

		if (rank == 0) {
			int* tmp = new int[Size];
			int *result = new int[Size];
			for (int i = 0; i < Size; i++)
				result[i] = INT_MIN;

			for (int i = 1; i < size; i++){
				MPI_Send(&Size, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(matrix, Size*Size, MPI_INT, i, 0, MPI_COMM_WORLD);
			}

			for (int i = 1; i < size; i++){
				MPI_Recv(tmp, Size, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
				for (int j = 0; j < Size; j++){
					if (tmp[j] > result[j])
						result[j] = tmp[j];
				}
			}

			cout << "Result = ";
			for (int i = 0; i < Size; i++)
				cout << result[i] << " ";
			cout << endl;

			delete matrix;
			delete tmp;
			delete result;
		}

		else {
			MPI_Recv(&Size, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
			
			matrix = createMatrix(Size);
			int* tmp = new int[Size];
			MPI_Recv(matrix, Size*Size, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &Status);
			for (int i = 0; i < Size; i++){
				tmp[i] = INT_MIN;
				for (int j = (rank - 1); j < Size; j += (size - 1))
				if (matrix[i * Size + j] > tmp[i])
					tmp[i] = matrix[i * Size + j];
			}
			MPI_Send(tmp, Size, MPI_INT, 0, 0, MPI_COMM_WORLD);
		}

	}

	if (size <= Size){
		MPI_Scatter(matrix, Size * partSize, MPI_INT, vec, Size*partSize, MPI_INT, 0, MPI_COMM_WORLD);

		int *localMax = new int[partSize];

		for (int i = 0; i < partSize; i++) {
			int max = INT_MIN;
			for (int j = 0; j < Size; j++) {
				if (vec[i * Size + j] > max) {
					max = vec[i * Size + j];
				}
			}
			localMax[i] = max;
		}
		int *totalMax = nullptr;
		if (rank == 0) {
			totalMax = new int[Size];
		}
		MPI_Gather(localMax, partSize, MPI_INT, totalMax, partSize, MPI_INT, 0, MPI_COMM_WORLD);

		if (rank == 0) {
			int tail = Size - size*partSize;
			for (int i = tail + 1; i < Size; i++) {
				int max = INT_MIN;
				for (int j = 0; j < Size; j++) {
					max = maxSearch(matrix[i*Size + j], max);
				}
				totalMax[i] = max;
			}
			cout << "Result: ";
			for (int i = 0; i < Size; i++) {
				cout << totalMax[i] << " ";
			}
		}
	}
	


	MPI_Finalize();
	return 0;
}
