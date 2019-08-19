<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Calc.aspx.cs" Inherits="ProjectCollection.Calc" %>
<%@ Register TagPrefix="WebForm" TagName="UserControl" Src="~/WebFormUserControl.ascx" %>

<!DOCTYPE html>

<html xmlns="http://www.w3.org/1999/xhtml">
<head runat="server">
    <title></title>
    <script src="https://ajax.aspnetcdn.com/ajax/jQuery/jquery-3.2.1.min.js"></script>
    <link rel="stylesheet" type="text/css" href="Content/calcStyles.css"/>
    <script src="Scripts\calcJS.js"></script>  
</head>
 
<body onload="getCookie()" onbeforeunload="setCookie()"> 
        <form id="form1" runat="server">
        <div class="scriptManager">
            <asp:ScriptManager ID="ScriptManager1" runat="server" />
        </div>
        <WebForm:UserControl runat="server" />
        <asp:HiddenField ID="convertFromVal" runat="server" Value="Bin" ClientIDMode="Static" />
        <asp:RequiredFieldValidator ID="reqValidator" runat="server" ErrorMessage="RequiredFieldValidator" controltovalidate="Text1" ClientIDMode="Static" SetFocusOnError="True"></asp:RequiredFieldValidator>
        <asp:HiddenField ID="convertToVal" runat="server" Value="Bin" ClientIDMode="Static" />
        <asp:HiddenField ID="doneGettingCookies" runat="server" Value="false" ClientIDMode="Static" />
        <h1 id="mainHeader">Calculator</h1>
            <div id="mainContainer">
                <asp:Panel ID="Panel1" runat="server" DefaultButton="calculateButton">
                    <asp:UpdatePanel ID="UpdatePanel1" runat="server" UpdateMode="Conditional">
                        <ContentTemplate>
                            <div id="textContainer">
                                <asp:TextBox ID="TextArea1" runat="server" TextMode="MultiLine" ClientIDMode="Static"></asp:TextBox>
                            </div>
                            <div id="inputContainer">
                                <asp:TextBox ID="Text1" runat="server" ClientIDMode="Static" AutoCompleteType="Disabled"></asp:TextBox>
                                <asp:Button ID="calculateButton" OnClick="calculateClick" OnClientClick="disableBtn()" runat="server" Text="Calculate" ClientIDMode="Static" UseSubmitBehavior="False" />                   
                            </div>
                        </ContentTemplate>
                    </asp:UpdatePanel>
                </asp:Panel>
                <div id="regexContainer">
                    <asp:RegularExpressionValidator ID="inputRegex" runat="server" ErrorMessage="Bad input or number of characters exceeded 100" controltovalidate="Text1" ClientIDMode="Static" validationexpression="^[a-zA-Z\d\s%\(\)\*\/\^\+\,\.\-]{0,100}$" SetFocusOnError="True"/>
                </div>
                <div id="controlsToggle">
                        <img src="Icons\downarrow.png" id="controlsIcon" />
                    </div>
                <div id="calcControls">
                    <div id="histButtonGroup">
                        <input id="Button1" type="button" value="Clear Display" />
                        <input id="Button2" type="button" value="Clear Session" />
                        <input id="Button3" type="button" value="Clear History" />
                    </div>
                     <div id="typeRadioGroup">
                         <asp:RadioButtonList runat="server" ID="radioButtons" ClientIDMode="Static" RepeatLayout="Flow">
                            <asp:ListItem Enabled="False" Text="Prefix"></asp:ListItem>
                            <asp:ListItem Selected="True" Text="Infix"></asp:ListItem>
                            <asp:ListItem Text="Postfix"></asp:ListItem>
                            <asp:ListItem Text="Converter"></asp:ListItem>
                         </asp:RadioButtonList>
                     </div>
                    <div id="convertOptions">
                        <label id="convertOptionsLabel1">To: </label>
                        <div class="dropDown">
                            <input id="convertTo" type="button" value="Bin&#x25BC"/>
                            <div id="convertToDD" class="DDContent">
                                <label>Bin</label>
                                <label>Oct</label>
                                <label>Dec</label>
                                <label>Hex</label>
                            </div>
                        </div>
                        <label id="convertOptionsLabel2">From: </label>
                        <div class="dropDown">
                            <input id="convertFrom" type="button" value="Bin&#x25BC;"/>
                            <div id="convertFromDD" class="DDContent">
                                <label>Bin</label>
                                <label>Oct</label>
                                <label>Dec</label>
                                <label>Hex</label>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
    </form>
</body>
</html>
