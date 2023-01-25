#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

static int num_neigh;
static int *neigh;

void msj(int rank1, int rank2)
{
    printf("M(%d,%d)\n", rank1, rank2);
}

void read_neighbours(int rank) {
// functie in care coordonatori 0,1,2,3 citesc din fisiere
  
    char fileName[13];
    sprintf(fileName, "cluster%d.txt", rank);

    ifstream fin(fileName);
	
    fin >> num_neigh;
    
    neigh = (int*)calloc(num_neigh + 2, sizeof(int));
    if(rank == 0){
        //neigh[0] = 1;
        neigh[1] = 3;
    }
    if(rank == 1){
        //neigh[0] = 0;
        neigh[1] = 2;
    }
    if(rank == 2){
        neigh[0] = 1;
        neigh[1] = 3;
    }
    if(rank == 3){
        neigh[0] = 0;
        neigh[1] = 2;
    }
// instiintez worker-ul citit ca ii sunt coordonator
	for (int i = 2; i < num_neigh + 2; i++){
        fin >> neigh[i];
        MPI_Send(&rank, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
        msj(rank, neigh[i]);
    }
    
    

}

// pt workeri aleg leader-ul ca fiind coordonatorul acestuia
int leader_chosing(int rank) {
	int leader;
    MPI_Status status;
    // workerii il vor avea ca lider/parinte/vecin doar pe coordonatorul sau
    MPI_Recv(&leader, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
    num_neigh = 1;
    neigh = (int*)calloc(1, sizeof(int));
	neigh[0] = leader;
    return leader;
}

int ** get_topology(int rank, int nProcesses, int leader) {
	int ** topology = (int**) calloc(sizeof(int*), nProcesses);
	int ** vTopology = (int**) calloc(sizeof(int*), nProcesses);
	
	for (int i = 0; i < nProcesses; i++) {
		topology[i] =(int*) calloc(sizeof(int), nProcesses);
		vTopology[i] = (int*)calloc(sizeof(int), nProcesses);
        // in vTopology o sa primesc topologia si o sa o updatez in topology
	}
    // adaug in topologie legatura dintre coordonatori si workeri
	for (int i = 2; i < num_neigh + 2; i++) {
		topology[rank][neigh[i]] = 1;
	}
	MPI_Status status;
	// Primim informatii de la toti copiii si actualizam matricea de topologie
    // 0 este parinte/leader pentru workerii sai +  coord 3
    // 3 este parinte pentru workerii sai +  coord 2
    // 2 este parinte pentru workerii sai +  coord 1    
        if(rank == 3){
            for(int i = 1; i < num_neigh + 2; i++){
                for(int j = 0; j < nProcesses; j++){
				MPI_Recv(vTopology[j], nProcesses, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
				for(int k = 0; k < nProcesses; k++){
					if(topology[j][k] == 0){
						topology[j][k] = vTopology[j][k];
					}
				}
            }
			}
        }else if(rank == 2){
			for(int i = 0; i < num_neigh + 2; i++){
                if(i == 1)continue;
                for(int j = 0; j < nProcesses; j++){
				MPI_Recv(vTopology[j], nProcesses, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
				for(int k = 0; k < nProcesses; k++){
					if(topology[j][k] == 0){
						topology[j][k] = vTopology[j][k];
					}
				}
            }
			}
		} else if(rank == 0){
            for(int i = 1; i < num_neigh + 2; i++){
                for(int j = 0; j < nProcesses; j++){
				MPI_Recv(vTopology[j], nProcesses, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
				for(int k = 0; k < nProcesses; k++){
					if(topology[j][k] == 0){
						topology[j][k] = vTopology[j][k];
					}
				}
			}
            }
        }
        else if(rank == 1){
            for(int i = 2; i < num_neigh + 2; i++){
            for(int j = 0; j < nProcesses; j++){
                 MPI_Recv(vTopology[j], nProcesses, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
            for(int k = 0; k < nProcesses; k++){
					if(topology[j][k] == 0){
						topology[j][k] = vTopology[j][k];
					}
				}
            }
        }
        }

	

	// Propagam matricea proprie catre parinte 
    if(rank != 0){
    for(int i = 0; i < nProcesses; i++){
	    
		    MPI_Send(topology[i], nProcesses, MPI_INT, leader, 0, MPI_COMM_WORLD);
            msj(rank, leader);
	}
    }

	// Daca nu suntem liderul, asteptam topologia completa de la parinte
	if(rank != 0){
		for(int i = 0; i < nProcesses; i++){
			MPI_Recv(topology[i], nProcesses, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
		}
	}
	
	// Trimitem topologia completa copiilor
    if(rank == 3){
        for(int i = 1; i < num_neigh + 2; i++){
			for(int j = 0; j < nProcesses; j++){
				MPI_Send(topology[j], nProcesses, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
                msj(rank, neigh[i]);
			}
		}
    }else if(rank == 2){
        for(int i = 0; i < num_neigh + 2; i++){
            if(i == 1)continue;
			for(int j = 0; j < nProcesses; j++){
				MPI_Send(topology[j], nProcesses, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
                msj(rank, neigh[i]);
			}
		}
	} else if(rank == 1){
        for(int i = 2; i < num_neigh + 2; i++){   
        for(int j = 0; j < nProcesses; j++){
				MPI_Send(topology[j], nProcesses, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
                msj(rank, 2);
			}
    }
    } else if(rank == 0) {
        for(int i = 1; i < num_neigh + 2; i++){
        for(int j = 0; j < nProcesses; j++){
				MPI_Send(topology[j], nProcesses, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
                msj(rank, 3);
			}
        }
    }
    

	return topology;
}

void print_topology(int rank, int nProcesses, int **topology)
{
    // functie ce afiseaza matricea de topologie in formatul cerut
    cout << rank << " -> ";
    int first;
    for(int i = 0; i < 4; i++){
        first = 0;
        cout << i <<":";
        for(int j = 0; j < nProcesses; j++){
            if(topology[i][j] != 0){
                if(first == 0){
                    cout << j;
                    first = 1; 
                }else {
                    cout << "," << j;
                }
            }
        }
        cout << " ";
    }
    cout << endl;
}

int* precalc_clusters_multiplies(int N, int nProcesses, int *cl)
{
    // Calculez de cate elemente trebuie sa se ocupe fiecare cluster
    int cat = N / (nProcesses - 4);
    int rest = N % (nProcesses - 4);

    int *cl_to_do = (int*)calloc(4, sizeof(int));

    for(int i = 0; i < 4; i++){
        cl_to_do[i] = cl[i] * cat;
    }

    for(int i = 0; i < 4; i++){
        if(cl[i] >= rest){
            cl_to_do[i] += rest;
            rest = 0;
        }else {
            cl_to_do[i] += cl[i];
            rest -= cl[i];
        }
    }

    return cl_to_do;
}

int* precalc_workers_multiplies(int work, int nr_workers)
{
    // Calculez de cate elemente se ocupa fiecare worker in parte
    int cat = work / nr_workers;
    int rest = work % nr_workers;
    int *worker_to_to = (int*)calloc(nr_workers, sizeof(int));
    for(int i = 0; i < nr_workers; i++){
        if(rest > 0)
            worker_to_to[i] = cat + 1;
        else
            worker_to_to[i] = cat;
        rest -= 1; 
    }

    return worker_to_to;
}

int* get_vector(int N)
{
    // Formez vectorul initial care trebuie prelucrat de workeri
    int *v = (int*)calloc(N, sizeof(int));
    for(int i = 0; i < N; i++){
        v[i] = N - i - 1;
    }

    return v;
}

void update_vector(int *v, int *v_aux, int start, int end)
{
    // Updatez din v_aux in v pozitiile intre start si end-1
    for(int i = start; i < end; i++){
        v[i] = v_aux[i];
    }
}

int* multiply_vector(int rank, int leader, int nProcesses, int **topology, int N_leader)
{
    MPI_Status status;
    int *v;
    int *cl = (int*)calloc(4, sizeof(int));
    int start, end;
    int N;
    int *cl_to_do, *worker_to_do;
    if(rank > 0){
        cl_to_do = (int*)calloc(4, sizeof(int));
    }
    // Trimit din 0 catre workerii acestuia si coordonatorul 3
    if(rank == 0){
        N = N_leader;
        v = (int*)calloc(N_leader, sizeof(int));
        v = get_vector(N);
        
        for(int i = 0; i < 4; i++){
            for(int j = 4; j < nProcesses; j++){
                cl[i] += topology[i][j]; // nr de workeri pt cluster[i]
            }
        }
        int *cl_to_do = precalc_clusters_multiplies(N, nProcesses, cl);
        start = 0;
        end = 0;
        worker_to_do = precalc_workers_multiplies(cl_to_do[0], num_neigh);
        for(int i = 1; i < num_neigh + 2; i++){
            MPI_Send(&N, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            msj(rank, neigh[i]);
            MPI_Send(v, N, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            msj(rank, neigh[i]);
            if(i == 1){
                MPI_Send(cl_to_do, 4, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
                msj(rank, neigh[i]);
            } else {
                // In vectorul de vecini workerii incep de pe indicele 2
                // pe 0 si 1 se regasesc coordonatori vecini
                // iar worker_to_do incepe cu indicii de la 0
                end += worker_to_do[i - 2];
                MPI_Send(&start, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
                msj(rank, neigh[i]);
                MPI_Send(&end, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
                msj(rank, neigh[i]);
            }
            
        }
            
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // Primesc informatiile de la 0 pe ceilalti coordonatori si trimit mai departe
    // la coordonatori cu care sunt in legatura si inca n-au primit vectorul si la workeri
    if(rank >= 1 && rank <= 3){
        MPI_Recv(&N, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        v = (int*)calloc(N, sizeof(int));
        MPI_Recv(v, N, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(cl_to_do, 4, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        
        // pentru fiecare coordonator in parte calculez si pozitia de start
        // a primului worker pe care il coordoneaza, iar pozitia de end
        // se actualizeaza pe parcurs cand trimit catre toti workerii 
        if(rank == 3){
            start = end = cl_to_do[2] + cl_to_do[1] + cl_to_do[0];
            worker_to_do = precalc_workers_multiplies(cl_to_do[3], num_neigh);
            MPI_Send(&N, 1, MPI_INT, 2, 0, MPI_COMM_WORLD);
            msj(rank, 2);
            MPI_Send(v, N, MPI_INT, 2, 0, MPI_COMM_WORLD);
            msj(rank, 2);
            MPI_Send(cl_to_do, 4, MPI_INT, 2, 0, MPI_COMM_WORLD);
            msj(rank, 2);
        }

        if(rank == 2){
            start = end = cl_to_do[1] + cl_to_do[0];
            worker_to_do = precalc_workers_multiplies(cl_to_do[2], num_neigh);
            MPI_Send(&N, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
            msj(rank, 1);
            MPI_Send(v, N, MPI_INT, 1, 0, MPI_COMM_WORLD);
            msj(rank, 1);
            MPI_Send(cl_to_do, 4, MPI_INT, 1, 0, MPI_COMM_WORLD);
            msj(rank, 1);
        }
        if(rank == 1){
            start = end = cl_to_do[0];
            worker_to_do = precalc_workers_multiplies(cl_to_do[1], num_neigh);
        }
        // coordonatorii trimit vectorul si pozitia de start si end
        // ce reprezinta inceputul si sfarsitul partii de care trebuie
        // sa se ocupe
        for(int i = 2; i < num_neigh + 2; i++){
            MPI_Send(&N, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            msj(rank, neigh[i]);
            MPI_Send(v, N, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            msj(rank, neigh[i]);
            
            end = start + worker_to_do[i - 2];
 
            MPI_Send(&start, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            msj(rank, neigh[i]);
            MPI_Send(&end, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD);
            msj(rank, neigh[i]);
            
            start = end;
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Workerii primesc vectorul de la coordonatori si il modifica la
    // pozitiile indicate, dupa care il trimit inapoi modificat
    if(rank > 3){
        MPI_Recv(&N, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        v = (int*)calloc(N, sizeof(int));
        MPI_Recv(v, N, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        
        MPI_Recv(&start, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);
        MPI_Recv(&end, 1, MPI_INT, leader, 0, MPI_COMM_WORLD, &status);

        for(int k = start; k < end; k++){
            v[k] *= 5;
        }

        MPI_Send(v, N, MPI_INT, leader, 0, MPI_COMM_WORLD);
        msj(rank, leader);
        MPI_Send(&start, 1, MPI_INT, leader, 0, MPI_COMM_WORLD);
        msj(rank, leader);
        MPI_Send(&end, 1, MPI_INT, leader, 0, MPI_COMM_WORLD);
        msj(rank, leader);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Coordonatorii de clustere primesc vectori modificati in pozitiile
    // atribuite si ii updateaza pe ai lor pentru a ii trimite ulterior
    // catre parinte/leader, in final ajungand la coordonatorul 0
    if(rank >= 1 && rank <= 3){
        int *v_aux = (int*)calloc(N, sizeof(int));

        for(int i = 2; i < num_neigh + 2; i++){
            MPI_Recv(v_aux, N, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&start, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&end, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
            update_vector(v, v_aux, start, end);
        }
        if(rank == 2){
            
            MPI_Recv(v_aux, N, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&start, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&end, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &status);
            update_vector(v, v_aux, start, end);
            int start_aux = cl_to_do[1] + cl_to_do[0];
            int end_aux = start_aux + cl_to_do[2];
            end = end_aux;
        }
        if(rank == 3){
            
            MPI_Recv(v_aux, N, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&start, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&end, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, &status);
            update_vector(v, v_aux, start, end);
            int start_aux = cl_to_do[2] + cl_to_do[1] + cl_to_do[0];
            int end_aux = start_aux + cl_to_do[3];
            end = end_aux;
        }
        if(rank == 1){
            start = cl_to_do[0];
            end = start + cl_to_do[1];
        }
        MPI_Send(v, N, MPI_INT, leader, 0, MPI_COMM_WORLD);
        msj(rank, leader);
        MPI_Send(&start, 1, MPI_INT, leader, 0, MPI_COMM_WORLD);
        msj(rank, leader);
        MPI_Send(&end, 1, MPI_INT, leader, 0, MPI_COMM_WORLD);
        msj(rank, leader);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    // Coordonatorul 0 asambleaza vectorul final din raspunsurile de la
    // coord 3 si workerii acestuia pentru a obtine solutia
    if(rank == 0){
        int *v_aux = (int*)calloc(N, sizeof(int));
        for(int i = 1; i < num_neigh + 2; i++){
            MPI_Recv(v_aux, N, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&start, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
            MPI_Recv(&end, 1, MPI_INT, neigh[i], 0, MPI_COMM_WORLD, &status);
            update_vector(v, v_aux, start, end);
        }
        
    }
    MPI_Barrier(MPI_COMM_WORLD);
    return v;
} 

int main(int argc, char * argv[])
{
    int rank, nProcesses, leader;
    int **topology;
    int *v;
    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

    int N = atoi(argv[1]);

    if(rank >=0 && rank <= 3)
        read_neighbours(rank);

    MPI_Barrier(MPI_COMM_WORLD);
    // Se aleg liderii/parintii conform topologiei din
    // pdf pentru cazul task-ului 3
    if(rank > 3)
        leader = leader_chosing(rank);
    else if(rank == 1)leader = 2;
    else if(rank == 2)leader = 3;
    else if(rank == 3)leader = 0;
    else if(rank == 0)leader = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    topology = get_topology(rank, nProcesses, leader);
    MPI_Barrier(MPI_COMM_WORLD);
    
    print_topology(rank, nProcesses, topology);
    MPI_Barrier(MPI_COMM_WORLD);

    v = multiply_vector(rank, leader, nProcesses, topology, N);
    
    if(rank == 0){
        cout << "Rezultat: ";
        for(int i = 0; i < N; i++){
            cout << v[i] << " ";
        }
    }

    MPI_Finalize();
    return 0;
}