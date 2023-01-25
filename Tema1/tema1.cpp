#include <bits/stdc++.h>

using namespace std;




pthread_mutex_t my_mutex;
pthread_barrier_t my_barrier;

int nr_mapperi, nr_reduceri;
int filesNumber;
char inputFile[100];
char aux[100];
int *files_size;

vector<vector<vector<unsigned long long>>> partial_lists;


queue<string> files_queue;

bool binarySearch(unsigned long long nr, int power, unsigned long long start, unsigned long long end){
    if(nr == 1)return true; // 1 este putere perfecta pentru orice exponent
    unsigned long long mid = (start + end) / 2;
    if(start > end)return false;
    if(pow(mid, power) == nr)return true;
    // daca pow(mid,power) este mai mare decat nr caut in partea stanga, altfel
    // caut in partea dreapta
    if(pow(mid, power) > nr)return binarySearch(nr, power, start, mid - 1);
    else return binarySearch(nr, power, mid + 1, end);
}

void *f_mapper_reducer(void *arg){
    int id = *(int *)arg;
    if(id < nr_mapperi){
    while(!files_queue.empty()){
        string fileName;
        pthread_mutex_lock(&my_mutex);
         if(files_queue.empty() == false){
            fileName = files_queue.front();
            files_queue.pop();
         }
         else break;
            
        pthread_mutex_unlock(&my_mutex);
        ifstream f(fileName);
        unsigned long long x, nr;
        f >> nr;

        for(int k = 0; k < nr; k++){
            f >> x;
            // verific daca un numar este putere perfecta a exponentilor
            // de la 2 la nr_reduceri + 1
            for(int power=2; power <= nr_reduceri + 1; power++){
                if(binarySearch(x, power, 1, x)){
                    partial_lists[id][power].push_back(x);
                }
            }
                    
                 
            
            
        }
    f.close();
    }
    }

    pthread_barrier_wait(&my_barrier);

    if(id >= nr_mapperi){
        int nr = 0;
        unordered_map<unsigned long long, bool> reduced; 
    for(int k = 0; k < nr_mapperi; k++){
        for(int i = 0; i < partial_lists[k][id - nr_mapperi + 2].size(); i++){
            if(reduced.find(partial_lists[k][id-nr_mapperi + 2][i]) == reduced.end()){
                reduced[partial_lists[k][id-nr_mapperi + 2][i]] = true; 
            }
            // daca nu exista in map il adaug, altfel trec mai departe iar la final
            // reducer.size() reprezinta nr de puteri perfecte unice pentru un exponent
        }
            
    }
    int putere = id - nr_mapperi + 2;
    string str = "out";
    str.append(to_string(putere));
    str.append(".txt");
    ofstream fout(str);
    fout << reduced.size();
    fout.close();
    }

    pthread_exit(NULL);
}


int main(int argc, char *argv[])
{
    
     if (argc < 3) {
        perror("Lista parametrii incompleta\n");
        exit(-1);
    }
    nr_mapperi = atoi(argv[1]);
    nr_reduceri = atoi(argv[2]);
    strcpy(inputFile, argv[3]);
    ifstream f(inputFile);
    f >> filesNumber;
    f.get();
    
    for(int i = 0; i < filesNumber; i++){
        f.getline(aux, 100);
        string file = aux;

        files_queue.push(file);
    }
    
    partial_lists.resize(nr_mapperi + 1);
    for(int i = 0; i <= nr_mapperi; i++){
        partial_lists[i].resize(nr_reduceri + 3);
    }
    pthread_t threads[nr_mapperi + nr_reduceri];
    int arguments[nr_mapperi + nr_reduceri];
    int r;
    void *status;
    pthread_barrier_init(&my_barrier, NULL, nr_mapperi + nr_reduceri);
    for (int i = 0; i < nr_mapperi + nr_reduceri; i++) {
        arguments[i] = i;
		r = pthread_create(&threads[i], NULL, f_mapper_reducer, &arguments[i]);

		if (r) {
			printf("Eroare la crearea thread-ului %d\n", i);
			exit(-1);
		}
	}

    pthread_mutex_init(&my_mutex, NULL);

    for (int i = 0; i < nr_mapperi + nr_reduceri; i++) {
		r = pthread_join(threads[i], &status);
		
		
		if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", i);
			exit(-1);
		}
	}
    pthread_mutex_destroy(&my_mutex);
    pthread_barrier_destroy(&my_barrier);

    f.close();

    return 0;
}