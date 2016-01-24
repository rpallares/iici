package gui;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.util.ArrayList;

import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.DefaultTableModel;

import entrepot.OlapRequete.Operator;


public class QueryResultPanel extends JPanel {
	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;
	JLabel label;
JTable table;
DefaultTableModel tm;
	public QueryResultPanel() {
		super(new BorderLayout());
		label = new JLabel();
		tm=new DefaultTableModel();

		table= new JTable(tm);
        table.setPreferredScrollableViewportSize(new Dimension(500, 70));
        table.setFillsViewportHeight(true);


        //Create the scroll pane and add the table to it.
        JScrollPane scrollPane = new JScrollPane(table);

		this.add(scrollPane,BorderLayout.CENTER);
		this.add(label,BorderLayout.SOUTH);
	}

	public void setQuery(String st1, String st2, Operator op) {
		label.setText("<" + st1 + "," + st2 + "," + op + ">");
	}



	public void setQuery(ResultSet res) {
		try {
			ResultSetMetaData rm = res.getMetaData();
			System.out.println("here:" + rm.getColumnCount());
			String[] columnNames = new String[rm.getColumnCount()];
			for (int i = 1; i <= rm.getColumnCount(); i++) {
				columnNames[i - 1] = rm.getColumnName(i);
				System.out.print(rm.getColumnName(i) + "\t");// rm.getColumnName(i)
			}
			System.out.println();
			ArrayList<Object[]> dataL = new ArrayList<Object[]>();
			while (res.next()) {
				System.out.println("la");
				Object[] tmp = new Object[columnNames.length];
				for (int i = 1; i <= columnNames.length; i++) {
					tmp[i - 1] = res.getObject(i);
					System.out.print(res.getObject(i) + "\t");
				}
				dataL.add(tmp);
				System.out.println("here");
			}
			Object[][] data=new Object[dataL.size()][columnNames.length];
			for(int i=0;i<dataL.size();i++){
				data[i]=dataL.get(i);
			}
			//table=new  JTable(data,columnNames);
			tm.setDataVector(data, columnNames);
			System.out.println("fin");

		} catch (SQLException e) {
			e.printStackTrace();
		}
	}
}
