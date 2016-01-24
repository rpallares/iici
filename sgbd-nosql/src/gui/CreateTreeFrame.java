package gui;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.GridLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import entrepot.OlapArbre;
import entrepot.OlapArbre.NomExistantException;
import entrepot.SQLType;

/**
 * JFrame pour creer l'arbre olap puis lance une QueryFrame sur l'arbre 
 * @author adel
 *
 */
public class CreateTreeFrame extends JFrame implements ActionListener {

    private static final long serialVersionUID = 1L;
    private static String ADD_COMMAND = "add";
    private static String REMOVE_COMMAND = "remove";
    private static String DONE_COMMAND = "done";
    private CreateTreePanel treePanel;

    public CreateTreeFrame() {
        super();
        this.setVisible(true);
        this.setPreferredSize(new Dimension(400, 400));
        this.setDefaultCloseOperation(JFrame.DISPOSE_ON_CLOSE);
        // Create the components.
        treePanel = new CreateTreePanel();
        JButton addButton = new JButton("Add");
        addButton.setActionCommand(ADD_COMMAND);
        addButton.addActionListener(this);

        JButton removeButton = new JButton("Remove");
        removeButton.setActionCommand(REMOVE_COMMAND);
        removeButton.addActionListener(this);

        JButton clearButton = new JButton("done");
        clearButton.setActionCommand(DONE_COMMAND);
        clearButton.addActionListener(this);
        // Lay everything out.
        JPanel panelP = new JPanel(new BorderLayout());
        treePanel.setPreferredSize(new Dimension(300, 150));
        this.setContentPane(panelP);
        panelP.add(treePanel, BorderLayout.CENTER);

        JPanel panel = new JPanel(new GridLayout(0, 3));
        panel.add(addButton);
        panel.add(removeButton);
        panel.add(clearButton);
        panelP.add(panel, BorderLayout.SOUTH);
        this.pack();
    }

    public void actionPerformed(ActionEvent e) {
        String command = e.getActionCommand();
        if (ADD_COMMAND.equals(command)) {
            try {
                String name = getNodeName();
                SQLType sql = getSqlType();
                treePanel.addObject(new OlapArbre(name, sql));
            } catch (NomExistantException e1) {
                e1.printStackTrace();
            }
        } else if (REMOVE_COMMAND.equals(command)) {
            treePanel.removeCurrentNode();
        } else if (DONE_COMMAND.equals(command)) {
            if (MainFrame.ma_bd.is_connected()) {
                MainFrame.ma_bd.executeUpdate(treePanel.getTreeModel().toSQLStarSchema());
            }
            new QueryFrame(treePanel.getTreeModel());
            this.dispose();
        }
    }

    /**
     * fenetre modal pour avoir le nom du noeud
     * @return
     */
    public String getNodeName() {
        String s = null;
        do {
            s = (String) JOptionPane.showInputDialog(this, "select name",
                    "Customized Dialog", JOptionPane.PLAIN_MESSAGE, null, null,
                    "Type");
        } while (s == null || s.length() == 0);
        return s;
    }

    public SQLType getSqlType() {
        SQLType s = (SQLType) JOptionPane.showInputDialog(this,
                "select SQLType", "Customized Dialog",
                JOptionPane.PLAIN_MESSAGE, null, SQLType.values(), "Type");
        return s;
    }
    /*
    public static void main(String[] args) {

    new CreateTreeFrame();
    System.out.println("here");
    }
     */
}
