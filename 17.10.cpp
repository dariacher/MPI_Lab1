#include <iostream>
#include <mpi.h>
#include <ctime>
#include <string>
using namespace std;

void printMatrix(int *data, int row, int col) {
	if (row < 15 && col < 15) {
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < col; j++) {
				cout << data[i * col + j] << " ";
			}
			cout << endl;
		}
	}
}

int* createMatrix(int row, int col) {
	int *matrix;
	matrix = new int[row*col];
	return matrix;
}

void fullMatrix(int *matrix, int row, int col) {
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			matrix[i*col + j] = rand() % 100;
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
	int *matrix = nullptr;

	int *sendCounts = nullptr, *offset = nullptr, *recBuf = nullptr;
	int localBuf, tail;
	double time;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	const int rows = stoi(string(argv[1]));
	const int cols = stoi(string(argv[2]));


	if (rank == 0) {

		matrix = new int[rows * cols];
		fullMatrix(matrix, rows, cols);
		printMatrix(matrix, rows, cols);
	}
	time = MPI_Wtime();
	sendCounts = new int[size];
	offset = new int[size];
	localBuf = rows*cols / size;

	//элементы, оставшиеся после разделения между процессами
	tail = (rows*cols) % size;

	sendCounts[0] = localBuf + tail;
	offset[0] = 0;
	for (int i = 1; i < size; i++) {
		sendCounts[i] = localBuf;
		offset[i] = tail + i*localBuf;
	}
	recBuf = new int[sendCounts[rank]];

	MPI_Scatterv(matrix, sendCounts, offset, MPI_INT, recBuf, sendCounts[rank], MPI_INT, 0, MPI_COMM_WORLD);
