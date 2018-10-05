/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package robra.v2;

import com.opencsv.*;
import java.io.FileReader;
//import java.io.FileWriter;
//import java.util.ArrayList;
//import java.util.List;
//import java.awt.EventQueue;
//import java.awt.event.ActionEvent;
//import java.awt.event.ActionListener;
//import java.io.*;
//import javax.swing.*;

/**
 *
 * @author Paulita
 */
public class RobraV2 {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
       
        try {
        String archCSV = "C:\\Repos\\Robra-V2\\Robra-V2\\Archivos-testing\\ISO-Codes.csv";
        
//        Leer de a una las filas del csv
        try (CSVReader csvReader = new CSVReader(new FileReader(archCSV))) {
            String[] fila = null;
            while((fila = csvReader.readNext()) != null) {
                System.out.println(fila[0]
                        + " | " + fila[1]
                        + " |  " + fila[2]);
            }   
        }
//        Leer y obtener todas las filas juntas
//        CSVReader csvReader = new CSVReader(new FileReader(archCSV));
//        List<String[]> datos = csvReader.readAll();
        }
        catch (Exception ex){
        System.out.println(ex);
        }
        
//        Exporta archivo
//        try {
//            String [] pais = {"Spain", "ES", "ESP", "724", "Yes"};
//            String archCSV = "C:\\Repos\\Robra-V2\\Robra-V2\\Archivos-testing\\Para-crear.csv";
//            
//            CSVWriter writer = new CSVWriter(new FileWriter(archCSV));
//
//            writer.writeNext(pais);
//
//            writer.close();
//        }
//        catch (Exception ex){
//            System.out.println(ex);
//        }
      
    }
    
}
