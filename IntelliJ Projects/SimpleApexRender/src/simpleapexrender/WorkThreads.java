/*
 * Copyright (C) 2025 Global Graphics Software Ltd. All rights reserved.
 *
 */

package simpleapexrender;

import java.util.concurrent.*;
import java.util.function.*;

/**
 *
 * @author elwinm
 */
public class WorkThreads 
{
    // A synchronized and blocking queue.
    // 
    // The constructor indicates the maximum number
    // of items in the queue.
    //
    // When adding an item to a queue
    //      It will block if the queue is full.
    //
    // When taking an item from the queue
    //      If the queue has been marked completed
    //          it will return null
    //
    //      It will also block if the queue is empty
    //
    public static class WorkQueue<T>
    {
        public WorkQueue(int maxLength) throws Exception
        {
            if (maxLength == 0)
                throw new Exception("Bad parameter: maxLength must be positive");

            m_capacity  = maxLength;
            m_queue     = new LinkedBlockingDeque<> (maxLength);
            m_finished  = false;
        }
    
        // Add an item, blocking if space is unavailable
        public void Add (T item)
        {
            try
            {
                m_queue.putLast(item);
            }
            catch (InterruptedException ex)
            {
            }
        }

        // Get the next item, whatever it is
        public T GetNext()
        {
            T item = null;

            try
            {
                do
                {
                    item = m_queue.pollFirst(500, TimeUnit.MILLISECONDS);
                }
                while (! m_finished && (item == null));
            }
            catch (InterruptedException ex)
            {
                return item;
            }

            return item;
        }

        public int getCapacity ()
        {
            return m_capacity;
        }

        public boolean getQueueFinished ()
        {
            return m_finished;
        }

        public void setQueueFinished (boolean finished)
        {
            m_finished = finished;
        }
        
        private int                     m_capacity;
        private boolean                 m_finished;
        private LinkedBlockingDeque<T>  m_queue;
    };

    
    // Worker that processes messages and
    // calls the callback routine for each message.
    //
    // If the next item returned from the queue is null
    //   then it exits the processing loop.
    //
    // If the callback routine returns falls
    //   it will also exit the loop.
    public static class Worker<T>
    {
        public Worker (WorkQueue<T> inputQueue,
                       Predicate<T> processItem
                    )

        {
            m_inputQueue    = inputQueue;
            m_processItem   = processItem;
        }

        public void ProcessMessages ()
        {
            while (true)
            {
                T item = m_inputQueue.GetNext ();

                if (item == null)
                    return;

                if (! m_processItem.test (item))
                    return;
            }
        }

        private WorkQueue<T>    m_inputQueue;
        private Predicate<T>    m_processItem;
    };


    // Enable this to test the workthreads code
    // This also serves as a short example on how to use it.
    public static class WorkThreadsTest
    {
        public static class WorkItem
        {
            public WorkItem(String msg)
            {
                m_msg = msg;
            }

            public String m_msg;
        };

        public static void runTest()
        {
            try
            {
                // Create the queue with a maximum number of items
                WorkQueue<WorkThreadsTest.WorkItem> workQueue = new WorkQueue<WorkThreadsTest.WorkItem> (10);

                System.out.printf("Created Work Queue with a bounded capacity of %d\n", workQueue.getCapacity());
                System.out.printf("Creating worker threads\n");

                // Create the worker thread whose callback just prints
                // out the worker Id and the WorkItem message
                Thread[] threads = new Thread[4];
                for (int i = 0; i < threads.length; i++)
                {
                    // It's strange, if I put this string inside the delegate
                    // it doesn't evaluate i immediately.
                    // The value if i is always 4.
                    // So now we immediately evaluate inside the loop
                    // so the variable fmt is used inside the delegate.
                    String fmt = String.format ("Worker %d got message %%s\n", i);

                    Predicate<WorkItem> processItemCB = (WorkItem item) ->
                    {
                        try
                        {
                            System.out.printf(fmt, item.m_msg);

                            // Introduce a sleep to simulate something being done that may
                            // take a while
                            Thread.sleep (250);
                        }
                        catch (InterruptedException e)
                        {
                            return false;
                        }
                        return true;
                    };

                    Worker<WorkItem> worker = new Worker<WorkItem> (workQueue, processItemCB);

                    Runnable runnable = () ->
                    {
                        worker.ProcessMessages ();
                    };

                    Thread thread = new Thread(runnable);

                    threads [i] = thread;

                    // Immediately start it
                    thread.start();
                }

                System.out.printf("Queueing up messages\n");

                // Queue up a bunch of messages.
                // Since the threads have already been started and are waiting
                // then the first 4 will get processed immediately.
                for (int i = 0; i < 100; i++)
                {
                    String label = String.format("Message %d", i);
                    WorkThreadsTest.WorkItem workItem = new WorkThreadsTest.WorkItem (label);

                    // And add to the message queue
                    workQueue.Add(workItem);
                }

                System.out.printf("Finished submitting all messages\n");

                // Let everyone know that no more items are coming
                workQueue.setQueueFinished(true);

                System.out.printf("Queue marked as finished\n");

                // And then wait for them to exit
                System.out.printf("Waiting for threads to exit.\n");
                for (var thread : threads)
                {
                    thread.join();
                }

                System.out.printf ("Finished all threads\n");
            }
            catch (InterruptedException e)
            {
                System.out.printf("Got exception %s\n", e.toString());
            }
            catch (Exception e)
            {
                System.out.printf("Got exception %s\n", e.toString());
            }
        }
    }
}
