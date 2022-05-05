using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace renga_surfaces
{
    public class Init : Renga.IPlugin
    {
        private List<Renga.ActionEventSource> m_eventSources = new List<Renga.ActionEventSource>();

        public bool Initialize(string pluginFolder)
        {
            var app = new Renga.Application();
            var ui = app.UI;
            var panelExtension = ui.CreateUIPanelExtension();

            //Create tool-down structure for plugins actions
            var dropDownButton = ui.CreateDropDownButton();
            dropDownButton.ToolTip = "Plugin's actions";

            //Create button for converting LandXml to IFC
            Renga.IAction to_convert = ui.CreateAction();
            to_convert.DisplayName = "Convert LandXml to IFC";
            Renga.ActionEventSource button_event = new Renga.ActionEventSource(to_convert);
            button_event.Triggered += (s, e) =>
            {
                //string landxml_path = null;
                //ui.ShowMessageBox(Renga.MessageIcon.MessageIcon_Info, "Message from Sample", "Context menu clicked");
                //select_file();
                //void select_file()
                //{
                //    OpenFileDialog selection_file = new OpenFileDialog();
                //    selection_file.Title = "Выберите файл LandXML";
                //    //selection_file.Filter = "landxml files(*.xml)";
                //    if (selection_file.ShowDialog() == DialogResult.OK) landxml_path = selection_file.FileName;
                //    else if (!File.Exists(landxml_path)) select_file();
                //    else select_file();
                //}

                new Landxml2ifc(app.Project);
            };
            m_eventSources.Add(button_event);

            dropDownButton.AddAction(to_convert);
            panelExtension.AddDropDownButton(dropDownButton);


            ui.AddExtensionToPrimaryPanel(panelExtension);
            ui.AddExtensionToActionsPanel(panelExtension, Renga.ViewType.ViewType_View3D);

            return true;
        }

        public void Stop()
        {
            foreach (var eventSource in m_eventSources)
                eventSource.Dispose();

            m_eventSources.Clear();
        }
    }
}

