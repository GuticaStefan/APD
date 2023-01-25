import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.io.*;

public class Tema2 {

    public static void main(String[] args) {
        BufferedReader reader1;
        BufferedWriter writer1, writer2;
        Integer nr_threads;
        FileReader file1;
        FileWriter wfile1, wfile2;

        String folder;
        folder = args[0];
        nr_threads = Integer.valueOf(args[1]); // nr de threaduri maxim de pe un nivel (P)
        AtomicInteger inQueue1 = new AtomicInteger(0);
        ExecutorService tpe1 = Executors.newFixedThreadPool(nr_threads); // level 1 maxim P threaduri
        ExecutorService tpe2 = Executors.newFixedThreadPool(nr_threads); // level 2 maxim P threaduri
        try {
            file1 = new FileReader(folder + "/orders.txt");
            reader1 = new BufferedReader(file1);

            wfile1 = new FileWriter("orders_out.txt");
            wfile2 = new FileWriter("order_products_out.txt");

            writer1 = new BufferedWriter(wfile1);
            writer2 = new BufferedWriter(wfile2);
            // pornesc initial P threaduri de nivel 1
            for (int i = 0; i < nr_threads; i++) {
                inQueue1.incrementAndGet();
                tpe1.submit(new MyTask1(i, inQueue1, tpe1, tpe2, folder, reader1, writer1, writer2, nr_threads));
            }
            // cand threadurile de nivel 1 s-au terminat inseamna ca s-au terminat si cele
            // de nivel 2, asa ca opresc executorservice-urile
            while (inQueue1.get() > 0) {

            }
            tpe1.shutdown();
            tpe2.shutdown();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

    }

}