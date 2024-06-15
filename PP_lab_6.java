// № варіанту 7
// № схеми 7

// Спільний ресурс 1 CR1 (буфер обміну даними):
// Структура даних, що використовується у якості спільного ресурсу 1 - Стек у вигляді вектора
// Засіб взаємного виключення при доступі до спільного ресурсу 1 - Монітор

// Спільний ресурс 2 CR2:
// Засіб взаємного виключення при доступі до змінних спільного ресурсу 2 - М’ютекс

// Землянський Едуард КВ-22

import java.util.concurrent.CyclicBarrier; // for cyclic barrier
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.Semaphore; // for semaphores
import java.util.Random;
import java.util.concurrent.locks.*; // for mutex

class Global // sems for P2 and P5
{
	public static Random random = new Random();
	public static Semaphore thread_sem1 = new Semaphore(0, true);
	public static Semaphore thread_sem2 = new Semaphore(0, true);
	public static ReentrantLock mutex = new ReentrantLock();
}

class CR1
{
	public static final int MaxBufSize = 300;
	public static final int MinBufSize = 0;
	int buf[] = new int[MaxBufSize+1];
	int ind = 0;
	boolean IsEmpty = ind  == MinBufSize;
	boolean IsFull = ind == MaxBufSize;
	String Thread_name;

	CR2 MyCR2 = new CR2();;
	
	synchronized void consume(String Thread_name)
	{
		while (IsEmpty)
			try
			{
				wait();
			}
			catch (InterruptedException e)
			{
				System.out.println("InterruptedException");
			}
			
		System.out.println("Thread "+ Thread_name + " consumed " + buf[ind] + " from buf[" + ind + "]");
		buf[ind] = 0;
		ind--;
	
		IsEmpty = ind == MinBufSize;
		IsFull = false;
			
		notify();
	}

	synchronized void produce (String Thread_name)
	{
		while (IsFull){
			System.out.println("Producer " + Thread_name + " terminated the programm !");
			MyCR2.print();
			System.exit(0);
		}
			
		ind++;
		buf[ind] = Global.random.nextInt(-100, 100);
		System.out.println("Thread " + Thread_name + " produced " + buf[ind] + " into buf[" + ind + "]");

		IsFull = ind == MaxBufSize;
		IsEmpty = false;
			
		notify();
	}
}

class CR2 {
    // Global variables with starting 'zero' values
    public static byte byteVariable = 0;
    public static short shortVariable = 0;
    public static int intVariable = 0;
    public static long longVariable = 0L;
    public static float floatVariable = 0.0f;
    public static double doubleVariable = 0.0;
    public static boolean booleanVariable = false;
    public static char charVariable = ' ';

    // Method to access variable values
    void access(String Thread_name) {
		System.out.println("Thread " + Thread_name + " accessed CR2 data (used)");
    }

	// Method to print variable values
    public void print() {
        System.out.println("byteVariable: " + byteVariable);
        System.out.println("shortVariable: " + shortVariable);
        System.out.println("intVariable: " + intVariable);
        System.out.println("longVariable: " + longVariable);
        System.out.println("floatVariable: " + floatVariable);
        System.out.println("doubleVariable: " + doubleVariable);
        System.out.println("booleanVariable: " + booleanVariable);
        System.out.println("charVariable: " + charVariable);
    }
	// Method to edit variable values

	void edit (String Thread_name){

		System.out.println("Thread " + Thread_name + " accessed CR2 data (edited)");

		byteVariable = (byte) Global.random.nextInt(Byte.MAX_VALUE + 1);
		shortVariable = (short) Global.random.nextInt(Short.MAX_VALUE + 1);
		intVariable = Global.random.nextInt();
		longVariable = Global.random.nextLong();
		floatVariable = Global.random.nextFloat();
		doubleVariable = Global.random.nextDouble();
		booleanVariable = !booleanVariable;
		int randomAscii = Global.random.nextInt(26) + 97;
        charVariable = (char) randomAscii;

	}
}

class P1 implements Runnable // ready
{
	Thread t;
	CR1 CR;

	P1 (CR1 CR1_arg)
	{
		this.CR = CR1_arg;
		t = new Thread (this, "P1");
		t.start();
	}
	
	public void run()
	{
		while (true) 
		{

			CR.produce(t.getName());
		}
	}
}

class P2 implements Runnable // ready
{
	Thread t;
	private CyclicBarrier br1;
	CR1 CR;

	P2 (CR1 CR_arg, CyclicBarrier brInit)
	{
		this.CR = CR_arg;
		t = new Thread (this, "P2");
		br1 = brInit;
		t.start();
	}
	
	public void run()
	{
		while (true)
		{
			// full sync with P3 via cyclic barr

			try{
			  	br1.await();
			}
			catch(BrokenBarrierException e){
			   	System.out.println(e.getMessage());
		    }
		    catch(InterruptedException e){
			  	System.out.println(e.getMessage());
		  	}

			// sem1 + sem2 full sync

			System.out.println("Thread P2 released semaphore for P5");
			Global.thread_sem2.release();

			try{
				Global.thread_sem1.acquire();
		  	}
			catch(InterruptedException e){
				System.out.println("Thread_1 interrupted");
		  	}
			System.out.println("Thread P2 got semaphore released from P5");

			// acces to CR1 (taking)

			CR.consume(t.getName());
		}
	}
}
class P3 implements Runnable
{
	Thread t;
	private CyclicBarrier br1;
	private CyclicBarrier br2;
	CR2 CR;

	P3 (CR2 CR_arg, CyclicBarrier brInit1, CyclicBarrier brInit2)
	{
		this.CR = CR_arg;
		t  = new Thread (this, "P3");
		br1 = brInit1;
		br2 = brInit2;
		t.start();
	}
	
	public void run()
	{
		while (true) 
		{
			// full sync with P2 via cyclic barr

			try{
			  	br1.await();
			}
			catch(BrokenBarrierException e){
			   	System.out.println(e.getMessage());
		    }
		    catch(InterruptedException e){
			  	System.out.println(e.getMessage());
		  	}

			// acessing CR2 with mutex

			Global.mutex.lock();
			CR.access(t.getName());
			Global.mutex.unlock();
			
			// full sync with P6 via cyclic barr

			try{
				br2.await();
		  	}
		  	catch(BrokenBarrierException e){
				System.out.println(e.getMessage());
		  	}
		  	catch(InterruptedException e){
				System.out.println(e.getMessage());
			}
		}
	}
}


class P4 implements Runnable
{
	Thread t;
	CR1 CR;

	P4 (CR1 CR_arg)
	{
		this.CR = CR_arg;
		t = new Thread (this, "P4");
		t.start();
	}
	
	public void run()
	{
		while (true) 
		{
			// acces to CR1 (loading)
			CR.produce(t.getName());
		}
	}
}

class P5 implements Runnable
{
	Thread t;
	CR1 CR1;
	CR2 CR2;

	P5 (CR1 CR1_arg, CR2 CR2_arg)
	{
		this.CR1 = CR1_arg;
		this.CR2 = CR2_arg;
		t = new Thread (this, "P5");
		t.start();
	}
	
	public void run()
	{
		while (true) 
		{
			// sem1 + sem2 full sync
			System.out.println("Thread P5 released semaphore for P2");
			Global.thread_sem1.release();

			try{
				Global.thread_sem2.acquire();
		  	}
			catch(InterruptedException e){
 				System.out.println("Thread_2 interrupted");
 		  	}
			System.out.println("Thread P5 got semaphore released from P2");

			// acces to CR1 (taking)

			CR1.consume(t.getName());
			
			// acces to CR2 (using)

			Global.mutex.lock();
			CR2.access(t.getName());
			Global.mutex.unlock();
		}
	}
}

class P6 implements Runnable
{
	Thread t;
	private CyclicBarrier br2;
	CR2 CR;

	P6 (CR2 CR_arg, CyclicBarrier brInit2)
	{
		this.CR = CR_arg;
		t = new Thread (this, "P6");
		br2 = brInit2;
		t.start();
	}
	
	public void run()
	{
		while (true) 
		{
			// full sync with P6 via cyclic barr
			try{
				br2.await();
		  	}
		  	catch(BrokenBarrierException e){
				 System.out.println(e.getMessage());
		  	}
		  	catch(InterruptedException e){
				System.out.println(e.getMessage());
			}

			// accesing CR2 (editing) with mutex
			Global.mutex.lock();
			CR.edit(t.getName());
			Global.mutex.unlock();

		}
	}
}

class Main {
    public static void main(String args[]) {

		CyclicBarrier br1 = new CyclicBarrier(2);
		CyclicBarrier br2 = new CyclicBarrier(2);

        // Creating an instance of CR1 and CR2
        CR1 MyCR1 = new CR1();
		CR2 MyCR2 = new CR2();

		MyCR2.print();

		new P1 (MyCR1);
		new P2 (MyCR1, br2);
		new P3 (MyCR2, br1, br2);
		new P4 (MyCR1);
		new P5 (MyCR1, MyCR2);
		new P6 (MyCR2, br2);
    }
}
