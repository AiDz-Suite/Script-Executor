using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Diagnostics.Eventing.Reader;
using System.Windows;

// I don't really know C# that well so code might be ass.
namespace aidsui
{
    class everything
    {
        const int port = 42069;

        public void send_script(string script)
        {
            using (Socket client_socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp))
            {
                client_socket.Connect("127.0.0.1", port);

                byte[] data = Encoding.ASCII.GetBytes(script);

                int sent = 0;
                while (sent < data.Length)
                {
                    sent += client_socket.Send(data, sent, data.Length - sent, SocketFlags.None);
                }

                /*
                                byte[] received_data = new byte[0x4000];

                                while (client_socket.Receive(received_data) != 0)
                                {
                                    response += Encoding.ASCII.GetString(received_data);
                                }
                */

                client_socket.Disconnect(false);
            }
        }
    }
}