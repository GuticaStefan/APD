import java.util.concurrent.ExecutorService;
import java.util.concurrent.atomic.AtomicInteger;
import java.io.*;

public class MyTask2 implements Runnable {

    AtomicInteger nr_produse;
    String command;
    BufferedReader reader;
    AtomicInteger inQueue;
    ExecutorService tpe;
    BufferedWriter writer2;

    public MyTask2(AtomicInteger nr_produse, String command, BufferedReader reader, AtomicInteger inQueue,
            ExecutorService tpe, BufferedWriter writer2) {
        this.nr_produse = nr_produse;
        this.command = command;
        this.reader = reader;
        this.inQueue = inQueue;
        this.tpe = tpe;
        this.writer2 = writer2;
    }

    @Override
    public void run() {
        while (true) {
            try {
                if (reader.ready()) {
                    String entireLine = reader.readLine();
                    if (entireLine == null)
                        break;
                    String currentCommand = entireLine.split(",")[0];

                    if (command.equals(currentCommand)) {
                        // daca id-ul comenzii de care ma ocup este acelasi cu cel
                        // citit il marchez ca si expediat
                        nr_produse.decrementAndGet();
                        writer2.write(entireLine + ",shipped\n");
                        writer2.flush();
                        if (nr_produse.get() <= 0) {

                            break;
                        }

                    }
                } else {

                    break;
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        inQueue.decrementAndGet();

    }
}