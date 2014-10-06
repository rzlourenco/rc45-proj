/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package server;

import java.io.*;
import java.net.*;

/**
 *
 * @author FMCalisto
 */
public class FileOperation {

    public void receive(Socket s, ObjectInputStream ois, ObjectOutputStream oos, InputStream is) {
        try {
            //System.out.println("Connected with: " + sock + " Receiving file.");
            //get file size
            long size = ois.readLong();
            System.out.println("Size: " + size);
            String name = (String) ois.readObject();//get file name            
            System.out.println("Name: " + name);
            //receive file
            String user = (String) ois.readObject();
            int bytesRead;
            int current = 0;
            byte[] mybytearray = new byte[(int) size];//byte writer
            File f = new File("d:\\ics4u\\" + user + "\\" + name);
            if (!f.exists()) {//generate a file if it does not exists.
                f.createNewFile();
            }
            FileOutputStream fos = new FileOutputStream(f.getAbsolutePath());//fos is for writing the file, bos is for buffering
            BufferedOutputStream bos = new BufferedOutputStream(fos);
            bytesRead = is.read(mybytearray, 0, mybytearray.length);//read from 0 to the length into byte array
            current = bytesRead;
            do {//buffer zone.
                bytesRead = is.read(mybytearray, current, (mybytearray.length - current));
                if (bytesRead >= 0) {
                    current += bytesRead;
                }
                System.out.println(current);
            } while (current != mybytearray.length || current == -1);
            System.out.println("pending to flush");
            bos.write(mybytearray, 0, current);
            bos.flush();
            bos.close();
            fos.close();
            oos.writeObject("1");//outputing number to indicate the process finished
        } catch (SocketException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    public void send(Socket s, ObjectInputStream ois, ObjectOutputStream oos, OutputStream os) {
        try {
            String name = (String) ois.readObject();//get name of the file
            System.out.println(name);
            String user = (String) ois.readObject();//get the user name to find directory
            System.out.println(user);
            File f = new File("d:\\ics4u\\" + user + "\\" + name);
            if (f.exists()) {
                long size = f.length();
                System.out.println(size);
                oos.writeObject(String.valueOf(size));//outputing the size of the file
                System.out.println("output length");
                //send file
                byte[] mybytearray = new byte[(int) f.length()];
                FileInputStream fis = new FileInputStream(f);//fis is for reading the file,bis is for buffering
                BufferedInputStream bis = new BufferedInputStream(fis);
                bis.read(mybytearray, 0, mybytearray.length);
                System.out.println("Sending...");
                os.write(mybytearray, 0, mybytearray.length);
                os.flush();
            }
        } catch (ConnectException e) {
            e.printStackTrace();
        }  catch (SocketException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}