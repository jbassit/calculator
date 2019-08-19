using System;
using System.Collections.Generic;
using System.Linq;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using System.Diagnostics;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Text.RegularExpressions;

namespace ProjectCollection
{
    static class CONST
    {
        public const string LOCAL_HOST_ADDR = "127.0.0.1";
        public const string CONNECTION_ERROR = "Connection failed";
        public const string CONNECTION_TIMEOUT = "Connection timedout";
        public const int DEFAULT_PORT = 6789;
        public const int BUFFER_SIZE = 2048;
        public const int SEND_RECV_TIMEOUT = 40 * 1000;
    }

    class Calculator_IPC
    {
        public string evaluate(string expr)
        {
            string result;

            try
            {
                connectToServer();
                sendRequest(expr);
                result = getResult();
            }
            catch (SocketException sockErr)
            {
                Debug.WriteLine(sockErr.ToString());
                if (sockErr.SocketErrorCode == SocketError.TimedOut)
                {
                    result = CONST.CONNECTION_TIMEOUT;
                }
                else
                {
                    result = CONST.CONNECTION_ERROR;
                }
            }
            finally
            {
                if (m_ServSocket.Connected)
                {
                    m_ServSocket.Shutdown(SocketShutdown.Both);
                }
                m_ServSocket.Close();
            }

            return result;
        }

        public static string toJSON(Dictionary<string, string> data)
        {
            string result = "{";
            int count = 0;

            if (data.Count == 0)
            {
                return "";
            }

            foreach (KeyValuePair<string, string> entry in data)
            {
                count++;
                result += "\"" + entry.Key + "\":\"" + entry.Value + ((count == data.Count) ? "\"}" : "\",");
            }

            return result;
        }


        private void connectToServer()
        {
            IPEndPoint destination = new IPEndPoint(IPAddress.Parse(CONST.LOCAL_HOST_ADDR), CONST.DEFAULT_PORT); // creates destination address
            m_ServSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp); // creates socket
            m_ServSocket.SendTimeout = CONST.SEND_RECV_TIMEOUT;
            m_ServSocket.ReceiveTimeout = CONST.SEND_RECV_TIMEOUT;
            m_ServSocket.Connect(destination); // attempts to connect socket with the destionation address
        }

        private void sendRequest(string expr)
        {
            byte[] endMsg = { 0 }; // ends message with null character
            byte[] msg = Encoding.ASCII.GetBytes(expr); // encodes message as ascii bytes
            m_ServSocket.Send(msg);
            m_ServSocket.Send(endMsg);
        }

        private string getResult()
        {
            int bytesRecieved = 0;
            string result = "";

            do
            {
                bytesRecieved = m_ServSocket.Receive(m_Buffer);
                result += Encoding.ASCII.GetString(m_Buffer, 0, bytesRecieved); // converts message from ascii bytes to string
            } while (m_ServSocket.Available > 0); // reads message

            return result;
        }

        private byte[] m_Buffer = new byte[CONST.BUFFER_SIZE];
        private Socket m_ServSocket;

    }

    public partial class Calc : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {
            string JQueryVer = "3.2.1";
            ScriptManager.ScriptResourceMapping.AddDefinition("jquery", new ScriptResourceDefinition // requirement for unobtrusive validation
            {
                Path = "~/Scripts/jquery-" + JQueryVer + ".min.js",
                DebugPath = "~/Scripts/jquery-" + JQueryVer + ".js",
                CdnPath = "http://ajax.aspnetcdn.com/ajax/jQuery/jquery-" + JQueryVer + ".min.js",
                CdnDebugPath = "http://ajax.aspnetcdn.com/ajax/jQuery/jquery-" + JQueryVer + ".js",
                CdnSupportsSecureConnection = true,
                LoadSuccessExpression = "window.jQuery"
            });

            if (TextArea1.Attributes["readonly"] == null) // makes text area read-only for the user
            {
                TextArea1.Attributes.Add("readonly", "readonly");
            }

            ScriptManager.RegisterStartupScript(this, GetType(), "histKeyEvents", "histKeyEvents()", true); // workaround for events associated with items within ajax panels
        }
        protected void calculateClick(object sender, EventArgs e)
        {
            Calculator_IPC bridge = new Calculator_IPC();
            ListItem selected = radioButtons.SelectedItem;
            Regex rmAllSpaces = new Regex(@"\s+");
            Regex rmTrailingSpaces = new Regex(@"\s+$");

            if (selected.Text == "Infix" || selected.Text == "Converter")
            {
                Text1.Text = rmAllSpaces.Replace(Text1.Text, "");
            }
            else
            {
                Text1.Text = rmTrailingSpaces.Replace(Text1.Text, "");
            }

            Dictionary<string, string> data = new Dictionary<string, string> // creates dictionary with appropriate calculator state
            {
                {"expr" , Text1.Text },
                {"mode", selected.Text},
                {"to", convertToVal.Value},
                {"from", convertFromVal.Value}
            };

            TextArea1.Text += Text1.Text + "\n   >> "; // writes input to display
            TextArea1.Text += bridge.evaluate(Calculator_IPC.toJSON(data)) + "\n\n"; // writes evaluated value to dispaly
            ScriptManager.RegisterStartupScript(this, GetType(), "scrollTextAreaToBottom", "slideDown()", true); // executes script for each postback to keep text area scrolled to bottom without flickering

            Text1.Text = "";
            Text1.Focus();
        }
    }
}