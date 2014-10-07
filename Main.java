/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package server;

import java.io.*;
import java.net.*;

import FileOperation;

/**
 *
 *  @author FMCalisto
 */

public class Main
{

    static int NG;

    static int CS_port = 58000 + NG;

    /**
     * @param args the command line arguments
     */

    public static void main(String[] args) throws IOException
    {
        ServerSocket ss = new ServerSocket(CS_port);//server port
        while (true)
        {
        //server should be constant working
            
            try
            {
                Socket socket = ss.accept();//get the connection
                System.out.println("Connected with:" + socket.getInetAddress());
                thread t = new thread(socket);//start a new thread to process the request of thi connection
            }

            catch (IOException e)
            {
                e.printStackTrace();
            }
        }
        // TODO code application logic here
    }
}

class thread implements Runnable {

    public Socket socket;
    public ObjectOutputStream oos;
    public ObjectInputStream ois;
    public InputStream is;
    public OutputStream os;

    public thread(Socket s) {
        socket = s;
        Thread t = new Thread(this);
        t.start();
    }

    public void run() {
        try {
            declare();//declarin ois,oos,os,is

            FileOperation fo = new FileOperation();
            
            while (true)
            {
                System.out.println("waiting for choice.");
                int choice = ois.readInt();//get a number from the client to process request
                if (choice == 1) {//choices are indicated as its name does
                    System.out.println("User registering.");
                } else if (choice == 2) {
                    System.out.println("User login.");
                } else if (choice == 3) {
                    System.out.println("User find password.");
                } else if (choice == 4) {
                    System.out.println("Receiving a file.");
                } else if (choice == 5) {
                    System.out.println("Sending a file.");
                    fo.send(socket, ois, oos, os);
                } else if (choice == 6) {
                    System.out.println("chaning info.");
                }

            }
        } catch (NullPointerException e) {//catch all the information
            e.printStackTrace();
        } catch (SocketException e) {
            System.out.println("Client disconected.");
            try {
                socket.close();
            } catch (IOException ex) {
                System.out.println("Socket close.");
            }
        } catch (EOFException e) {
            System.out.println("Client disconected.");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    /*
     * ois is for reading object, generally String, int,etc
     * oos if for outputing object
     * os is for outputing file stream
     * is is for inputing file stream
     */

    public void declare() {
        try {
            oos = new ObjectOutputStream(socket.getOutputStream());
            ois = new ObjectInputStream(socket.getInputStream());
            is = socket.getInputStream();
            os = socket.getOutputStream();
        } catch (SocketException e) {
            System.out.println("Client disconected.");
            try {
                socket.close();
            } catch (IOException ex) {
                System.out.println("Socket close.");
            }
        } catch (EOFException e) {
            System.out.println("Client disconected.");
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
