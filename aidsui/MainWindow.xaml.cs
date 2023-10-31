using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Diagnostics;
using System.Xml.Linq;

namespace aidsui
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        everything main = new everything();

        public MainWindow()
        {
            InitializeComponent();
        }

        private void Border_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            this.DragMove();
        }

        private void Label_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            Application.Current.Shutdown();
        }

        private void Label_MouseLeftButtonDown_1(object sender, MouseButtonEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        private void TMToggleClick(object sender, MouseButtonEventArgs e)
        {
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                main.send_script(scriptbox.Text);
            }
            catch (Exception except)
            {
                logs.Text += "Failed to send a script\nError: " + except.Message + "\n";
            }
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            logs.Text = "";
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            // Thanks!
            // https://stackoverflow.com/questions/240171/launching-an-application-exe-from-c

            ProcessStartInfo start = new ProcessStartInfo();
            start.FileName = Environment.CurrentDirectory + "\\AiDz Injector.exe";
            start.WorkingDirectory = Environment.CurrentDirectory;
            start.WindowStyle = ProcessWindowStyle.Hidden;
            start.CreateNoWindow = true;

            try
            {
                Process.Start(start);
            }
            catch (Exception except)
            {
                logs.Text += "Failed to inject: " + except.Message + "\n";
            }
        }
    }
}