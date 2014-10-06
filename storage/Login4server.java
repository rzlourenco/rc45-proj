/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package server;

import java.net.*;
import java.io.*;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 *
 * @author FMCalisto
 */
public class Login4server
{

    public Socket sock;

    public void changeinfo(ObjectInputStream ois, ObjectOutputStream oos) {
        try {
            String[] info = new String[7];
            for (int i = 0; i < 7; i++)
            {
                info[i] = (String) ois.readObject();//get the new info and store it into the array
            }
            FileWriter out = new FileWriter("d:/ics4u/" + info[0] + "/info/info.db");//info is stored in a file
            for (int i = 0; i < 7; i++) {
                    out.write(info[i]);
                    System.out.println("Write:" + info[i]);
                    if (i != 7) {
                        out.write("\r\n");
                    }
                }
                out.close();
        } catch (SocketException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }

    }

    public void login(Socket sock, ObjectInputStream ois, ObjectOutputStream oos) {
        try {
            String user = (String) ois.readObject();//getting the user name
            System.out.println(user);
            String password = (String) ois.readObject();//getting the password
            System.out.println(password);
            File f = new File("d:/ics4u/" + user + "/info/info.db");
            if (f.exists()) {//find if the user exist
                String[] info = getInfo(f.getAbsolutePath());
                if (info[1].equals(password)) {//find if password matches
                    oos.writeObject("1");
                    for (int i = 0; i < 7; i++) {//when sucessfully login, info is output
                        oos.writeObject(info[i]);
                        System.out.println(info[i]);
                    }
                    f = new File("d:/ics4u/" + user);
                    File[] f1 = f.listFiles();
                    oos.writeObject(f1.length - 1);
                    for (int i = 0; i < f1.length; i++) {//then the file list in storation is sent
                        if (!f1[i].getName().equals("info")) {
                            oos.writeObject(f1[i].getName());
                        }
                    }
                } else {
                    oos.writeObject("2");//password does not match
                }
            } else {
                oos.writeObject("3");//account does not exist
            }
        } catch (SocketException e) {
            e.printStackTrace();
        } catch (EOFException e) {
            System.out.println("Client disconected.");
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void findpass(Socket sock, ObjectInputStream ois, ObjectOutputStream oos) {
        try {
            String user = (String) ois.readObject();//getting the name of user
            System.out.println("User name." + user);
            File f = new File("d:/ics4u/" + user + "/info/info.db");
            if (f.exists()) {//check if the user exist
                System.out.println("Exists.");
                String[] info = getInfo(f.getAbsolutePath());
                oos.writeObject("true");
                oos.writeObject(info[5]);//sending the secure question and secure answer
                oos.writeObject(info[6]);
                oos.writeObject(info[1]);//password
                System.out.println("Finished outputing info.");
            } else {
                System.out.println("2");
                oos.writeObject("false");
                System.out.println("2");
            }
        } catch (SocketException e) {
            e.printStackTrace();
        } catch (EOFException e) {
            System.out.println("Client disconected.");
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }

    public void register(Socket sock, ObjectInputStream ois, ObjectOutputStream oos) {
        try {
            String[] info = new String[7];
            System.out.println("Getting info.");
            for (int i = 0; i < 7; i++) {
                info[i] = (String) ois.readObject();//getting info of the user
            }
            File f = new File("D:/ics4u/" + info[0]);
            System.out.println("Writing info.");
            boolean valid = false;
            if (f.exists()) {//check if user exist already
                valid = false;
                oos.writeObject("false");
            } else {
                valid = true;
                oos.writeObject("true");
            }
            //reply true or false to indicates first step
            System.out.println("Writing valid:" + valid);
            if (valid) {
                System.out.println("Writing info.");
                f.mkdir();
                f = new File("D:/ics4u/" + info[0] + "/info");
                f.mkdir();
                f = new File("D:/ics4u/" + info[0] + "/info/info.db");
                f.createNewFile();//creating a new folder for new user
                BufferedWriter out = new BufferedWriter(new FileWriter(f.getAbsolutePath()));
                for (int i = 0; i < 7; i++) {
                    out.write(info[i]);
                    System.out.println("Write:" + info[i]);//writing user info into the file
                    if (i != 7) {
                        out.write("\r\n");
                    }
                }
                out.close();
            }
        } catch (SocketException e) {
            e.printStackTrace();
        } catch (EOFException e) {
            System.out.println("Client disconected.");
        } catch (IOException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        }
    }
   //function of getting input
    public String[] getInfo(String path) {
        String[] info = new String[7];
        try {
            BufferedReader in = new BufferedReader(new FileReader(path));
            for (int i = 0; i < 7; i++) {
                info[i] = in.readLine();//get all the info from the file
            }
            in.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return info;
    }
}