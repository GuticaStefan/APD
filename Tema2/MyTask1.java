import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;

public class MyTask1 implements Runnable {
    int id;
    BufferedReader reader;
    AtomicInteger inQueue1, inQueue2;
    ExecutorService tpe1, tpe2;
    String folder;
    BufferedWriter writer1, writer2;
    Integer nr_threads;

    public MyTask1(int id, AtomicInteger inQueue1, ExecutorService tpe1, ExecutorService tpe2, String folder,
            BufferedReader reader, BufferedWriter writer1, BufferedWriter writer2, Integer nr_threads) {
        this.id = id;
        this.inQueue1 = inQueue1;
        this.tpe1 = tpe1;
        this.tpe2 = tpe2;
        this.folder = folder;
        this.writer1 = writer1;
        this.writer2 = writer2;
        this.reader = reader;
        this.nr_threads = nr_threads;
    }

    @Override
    public void run() {
        while (true) {

            try {
                // daca se poate citi (nu s-a ajuns la final sau nu citeste alt thread)
                if (reader.ready()) {
                    String entireLine = reader.readLine();
                    if (entireLine == null) // daca s-a ajuns la final cu un alt thread(fara aceasta conditie functiona
                                            // intotdeauna)
                        break;
                    String command = entireLine.split(",")[0];
                    AtomicInteger nr_produse = new AtomicInteger(Integer.valueOf(entireLine.split(",")[1]));
                    if (nr_produse.get() == 0) { // comanda de tip EmptyOrder
                        continue;
                    }

                    if (nr_produse.get() >= 0) {
                        FileReader file2 = new FileReader(folder + "/order_products.txt");
                        BufferedReader reader2 = new BufferedReader(file2);
                        // folosesc acelasi bufferedReader pt threaduri ce se ocupa de aceeasi comanda
                        // pentru a nu citi aceeasi linie de mai multe ori
                        inQueue2 = new AtomicInteger(0);// folosesc o noua coada pt fiecare comanda
                        // ca sa imi dau seama cand s-a finalizat o comanda si pot trece mai departe
                        int threads;
                        // in cazul in care nr de produse al unei comenzi este mai mic
                        // decat P nu este necesar sa pornesc P threaduri
                        if (nr_produse.get() < nr_threads) {
                            threads = nr_produse.get();

                        } else {
                            threads = nr_threads.intValue();
                        }

                        for (int i = 0; i < threads; i++) {
                            inQueue2.incrementAndGet();
                            tpe2.submit(new MyTask2(nr_produse, command, reader2, inQueue2, tpe2, writer2));
                        }

                        while (inQueue2.get() > 0) {

                        }
                        // cand se va iesi din while inseamna ca s-a expediat comanda si scriu in fisier
                        writer1.write(entireLine + ",shipped\n");
                        writer1.flush();
                    }

                } else {

                    break;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }

        }
        inQueue1.decrementAndGet();
    }

}