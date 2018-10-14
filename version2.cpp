#include <iostream>
#include <mpi.h>
#include <string>
using namespace std;

void printMatrix(int *data, int row, int col) {
	for (int i = 0; i < row; i++) {
		for (int j = 0; j < col; j++) {
			cout << data[i * col + j] << " ";
		}
		cout << endl;
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

int main(int argc, char **argv) {
	int rank, size;
	int rows, cols;
	int *matrix;
	int* data;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (rank == 0) {
		cout << "Enter the number of rows: ";
		cin >> rows;
		cout << endl;
		cout << "Enter the number of columns: ";
		cin >> cols;
		cout << endl;

		matrix = createMatrix(rows, cols);
		
		data = createMatrix(rows, cols);
		fullMatrix(matrix, rows, cols);
		printMatrix(matrix, rows, cols);
		for (int i = 0; i < rows; i++){
			for (int j = 0; j < cols; j++) {
				data[i*cols + rows] = matrix[i*cols + rows];
			}
		}
		

	}

	if (size > 1) {
		int partSize = cols / size;
		int tail = cols & size;

		int *col = new int[rows];
		int *max = new int[rows];

		for (int i = 0; i < cols - tail; i += partSize) {
			int displace;
			if (i == 0) {
				displace = 0;
			}
			else {
				displace = (i - 1) *rows;
			}
			int *send;
			if (rank == 0){
				send = data + displace;
			}
			else {
				send = nullptr;
			}
			
			max = rank == 0 ? matrix + rows*(i + partSize - 1) : nullptr;
			MPI_Scatter(send, rows, MPI_INT, col, rows, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Reduce(col, max, rows, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
		}

		if (tail > 0) {
			if (rank == 0) {
				for (int i = cols - tail, j = 1; i < cols && j <= tail; i++, j++) {
					MPI_Send(matrix + i*rows, rows, MPI_INT, j, 0, MPI_COMM_WORLD);
				}
				for (int i = 0; i < rows; i++) {
					col[i] = matrix[(cols - tail - 1) *rows + i];
				}
			}
			else if (rank <= tail) {
				MPI_Recv(col, rows, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			}
			else if (rank > tail) {
				for (int i = 0; i < rows; i++) {
					col[i] = INT_MIN;
				}
			}

			MPI_Barrier(MPI_COMM_WORLD);
			max = rank == 0 ? matrix + (cols - 1)* rows : nullptr;
			MPI_Reduce(col, max, rows, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);
		}

		if (rank == 0) {
			for (int i = 0; i < rows; i++){
				cout << max[i] << endl;
			}
		}
		MPI_Finalize();
	}
	return 0;
}
