using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;
using Renga;

namespace renga_surfaces
{
    public class Landxml2ifc
    {
        private double dX = 0d;
        private double dY = 0d;
        private double dZ = 0d;
        private double angle = 0d;
        private bool need_recalc = false;

        private List<Dictionary<int, Renga.FloatPoint3D>> list_surface_points = new List<Dictionary<int, FloatPoint3D>>();
        private List<List<int[]>> list_surface_triangles = new List<List<int[]>>();
        private List<string> list_surface_name = new List<string>();

        private string path_to_landxml_file = null;
        private Renga.IProject r_project;
        private string ifc_result_path = null;
        private string path_to_temp_dir = null;
        public Landxml2ifc(Renga.IProject r_project)
        {
            this.r_project = r_project;
            //this.path_to_landxml_file = path_to_landxml_file;
            GetSourceFile();
            //GetInternalOffset();
            GetInternalOffset2();
            //ReadLandXml();
            //WriteToIfc();
            //WriteToText();
            ILibraryImport lib_cpp = new LibraryImport_x64();
            int wait_result = lib_cpp.run_landxml2iges(path_to_landxml_file.ToCharArray());
        }
        /// <summary>
        /// No work in Renga (program problems)
        /// </summary>
        private void GetInternalOffset()
        {
            IBuildingInfo data = r_project.BuildingInfo;
            IPropertyContainer props = data.GetProperties();
            var props_ids = props.GetIds();
            if (props_ids != null)
            {
                string x_name = "IfcLocationX";
                string y_name = "IfcLocationY";
                string z_name = "IfcLocationZ";
                string angle_name = "IfcDirectionPrecession";

                bool for_x = false;
                bool for_y = false;
                bool for_z = false;
                bool for_angle = false;
                for (int counter_prop = 0; counter_prop < props_ids.Count; counter_prop++)
                {
                    IProperty one_prop = props.Get(props_ids.Get(counter_prop));
                    if (one_prop.Name == x_name && one_prop.Type == Renga.PropertyType.PropertyType_Double)
                    {
                        dX = one_prop.GetDoubleValue();
                        for_x = true;
                    }
                    if (one_prop.Name == y_name && one_prop.Type == Renga.PropertyType.PropertyType_Double)
                    {
                        dY = one_prop.GetDoubleValue();
                        for_y = true;
                    }
                    if (one_prop.Name == z_name && one_prop.Type == Renga.PropertyType.PropertyType_Double)
                    {
                        dZ = one_prop.GetDoubleValue();
                        for_z = true;
                    }
                    if (one_prop.Name == angle_name && one_prop.Type == Renga.PropertyType.PropertyType_Angle)
                    {
                        angle = one_prop.GetAngleValue(Renga.AngleUnit.AngleUnit_Degrees) / 180.0 * Math.PI;
                        for_angle = true;
                    }
                }
                if (for_x == true && for_y == true && for_z == true && for_angle == true) need_recalc = true;
                else need_recalc = false;
            }
            
        }
        private void GetInternalOffset2()
        {
            string path_to_offset_file = null;
            OpenFileDialog selection_file = new OpenFileDialog();
            selection_file.Title = "Выберите текстовый файл с параметрами сдвига (4 строки)";
            selection_file.Filter = "Текстовые файлы(*.txt)|*.txt";
            if (selection_file.ShowDialog() == DialogResult.OK) path_to_offset_file = selection_file.FileName;
            else if (!File.Exists(path_to_offset_file)) GetInternalOffset2();
            else GetInternalOffset2();

            string[] parameters_str = File.ReadAllLines(path_to_offset_file, Encoding.UTF8);
            //Console.WriteLine("Введите последовательно через строку параметры IfcLocationX, IfcLocationY, IfcLocationZ, IfcDirectionPrecession");
            dX = Double.Parse(parameters_str[0]) / 1000.0;
            dY = Double.Parse(parameters_str[1]) / 1000.0;
            dZ = Double.Parse(parameters_str[2]) / 1000.0;
            angle = Double.Parse(parameters_str[3]) / 180.0 * Math.PI;
            need_recalc = true;
            //Console.Clear();

        }
        private void GetSourceFile()
        {
            OpenFileDialog selection_file = new OpenFileDialog();
            selection_file.Title = "Выберите файл LandXML";
            selection_file.Filter = "LandXML файлы(*.xml)|*.xml";
            if (selection_file.ShowDialog() == DialogResult.OK) path_to_landxml_file = selection_file.FileName;
            else if (!File.Exists(path_to_landxml_file)) GetSourceFile();
            else GetSourceFile();

        }
        private void ReCalcPoints (ref double x, ref double y, ref double z)
        {
            x -= dX;
            y -= dY;
            z -= dZ;

            x = x * Math.Cos(angle) - y * Math.Sin(angle);
            y = x * Math.Sin(angle) + y * Math.Cos(angle);
            //z = z;
        }
        private void ReadLandXml()
        {
            XDocument xml = XDocument.Load(path_to_landxml_file);
            IEnumerable<XElement> surfaces = xml.Descendants().Where(a => a.Name.LocalName == "Surface");
            foreach (XElement surface in surfaces)
            {
                string surface_name = surface.Attribute("name").Value;
                list_surface_name.Add(surface_name);
                Dictionary<int, Renga.FloatPoint3D> surface_points = new Dictionary<int, FloatPoint3D>();
                List<int[]> surface_triangles = new List<int[]>();
                XElement points = surface.Descendants().Where(a => a.Name.LocalName == "Pnts").First();
                XElement triangles = surface.Descendants().Where(a => a.Name.LocalName == "Faces").First();

                foreach (XElement point in points.Elements())
                {
                    int point_id = Convert.ToInt32(point.Attribute("id").Value);
                    double[] point_coords = point.Value.Split(' ').Select(a => Convert.ToDouble(a)).ToArray();
                    if (need_recalc) ReCalcPoints(ref point_coords[1], ref point_coords[0], ref point_coords[2]);
                    FloatPoint3D new_point = new FloatPoint3D();
                    new_point.X = (float)point_coords[1];
                    new_point.Y = (float)point_coords[0];
                    new_point.Z = (float)point_coords[2];
                    surface_points.Add(point_id, new_point);
                }
                foreach (XElement triangle in triangles.Elements())
                {
                    int[] points_indexes = triangle.Value.Split(' ').Select(a => Convert.ToInt32(a)).ToArray();
                    surface_triangles.Add(points_indexes);
                }
                list_surface_points.Add(surface_points);
                list_surface_triangles.Add(surface_triangles);
            }
            
        }
        private void WriteToText()
        {
            path_to_temp_dir = new FileInfo(path_to_landxml_file).DirectoryName + "\\" + Guid.NewGuid().ToString();
            if (!Directory.Exists(path_to_temp_dir)) Directory.CreateDirectory(path_to_temp_dir);
            
            for (int counter_surface = 0; counter_surface < list_surface_name.Count; counter_surface++)
            {
                StringBuilder file_data = new StringBuilder();
                string surface_name = list_surface_name[counter_surface];
                Dictionary<int, Renga.FloatPoint3D> surface_points = list_surface_points[counter_surface];
                List<int[]> surface_triangles = list_surface_triangles[counter_surface];
                foreach (int[] triangle_definition in surface_triangles)
                {
                    foreach (int triangle_def_point in triangle_definition)
                    {
                        Renga.FloatPoint3D point = surface_points[triangle_def_point];
                        file_data.AppendLine($"{point.X * 1000.0};{point.Y * 1000.0};{point.Z * 1000.0}");
                    }
                    //file_data.AppendLine(Environment.NewLine);
                }
                File.WriteAllText(path_to_temp_dir + "\\" + surface_name + ".txt", file_data.ToString());
            }
        }

    }
}
