using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace renga_surfaces
{
    internal class LibraryImport_x64 : ILibraryImport
    {
        [DllImport("surface_geom", CallingConvention = CallingConvention.StdCall, ExactSpelling = false, EntryPoint = "run_landxml2iges")]
        private static extern int run_landxml2iges_internal(char[] dir_path);

        public int run_landxml2iges(char[] dir_path)
        {
            return run_landxml2iges_internal(dir_path);
        }
    }

    //[UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Ansi)]
    //public delegate void Notification(string value);
    public interface ILibraryImport
    {
        int run_landxml2iges(char[] dir_path);
    }
}
