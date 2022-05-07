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
        private Renga.IUI ui_current;
        Renga.IImage create_icon_image (string icon_path)
        {
            Renga.IImage image = ui_current.CreateImage();
            image.LoadFromFile(icon_path);
            return image;
        }

        public bool Initialize(string pluginFolder)
        {
            string plugin_dir = pluginFolder.Replace("/", "\\") + "\\";
            Renga.Application app = new Renga.Application();
            ui_current = app.UI;
            Renga.IUIPanelExtension panelExtension = ui_current.CreateUIPanelExtension();

            //Create tool-down structure for plugins actions
            Renga.IDropDownButton dropDownButton = ui_current.CreateDropDownButton();
            dropDownButton.ToolTip = "Plugin's actions";
            dropDownButton.Icon = create_icon_image(plugin_dir + "main_button.png");

            //Create button for converting LandXml to IFC
            Renga.IAction to_convert = ui_current.CreateAction();
            to_convert.Icon = create_icon_image(plugin_dir + "load_surface.png");
            to_convert.DisplayName = "Convert LandXml to IFC";
            Renga.ActionEventSource button_event = new Renga.ActionEventSource(to_convert);
            button_event.Triggered += (s, e) =>
            {
                new Landxml2ifc(app.Project); //Landxml2ifc operation = 
                //app.OpenProject(operation.ifc_result_path);
                ui_current.ShowMessageBox(Renga.MessageIcon.MessageIcon_Info, "Plugin message", "Операция успещно закончена!");

            };
            m_eventSources.Add(button_event);

            dropDownButton.AddAction(to_convert);
            panelExtension.AddDropDownButton(dropDownButton);


            ui_current.AddExtensionToPrimaryPanel(panelExtension);
            ui_current.AddExtensionToActionsPanel(panelExtension, Renga.ViewType.ViewType_View3D);

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

