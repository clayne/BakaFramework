import argparse, os, zipfile

def make_zipfile(a_name):
	return zipfile.ZipFile("{}.zip".format(a_name), "w", zipfile.ZIP_DEFLATED)

def make_rel_archive(a_name, a_args):
	zip = make_zipfile(a_name)
	def write(a_file, a_directory):
		zip.write(
			a_file,
			"{}/{}".format(
				a_directory,
				os.path.basename(a_file)))

	def write_direct(a_file, a_path):
		zip.write(a_file, a_path)

	for file in a_args.plugin_files:
		write(file, "F4SE/Plugins")

	if (a_args.config_files):
		for file in a_args.config_files:
			write_direct(file, "F4SE/Plugins/{}.toml".format(a_name))

	if (a_args.papyrus_pex_files):
		for each_file in a_args.papyrus_pex_files:
			files = each_file.split(";")
			for file in files:
				write(file, "Scripts")

	if (a_args.papyrus_psc_files):
		for each_file in a_args.papyrus_psc_files:
			files = each_file.split(";")
			for file in files:
				write(file, "Scripts/Source/User")

def make_dbg_archive(a_name, a_args):
	zip = make_zipfile("{}_pdbs".format(a_name))
	for pdb in a_args.pdbs:
		zip.write(pdb, os.path.basename(pdb))

def parse_arguments():
	parser = argparse.ArgumentParser(description="archive build artifacts for distribution")
	parser.add_argument("--bin-dir", type=str, help="the project binary directory", required=True)
	parser.add_argument("--papyrus-pex-files", type=str, help="the compiled papyrus files", nargs="+", required=False)
	parser.add_argument("--papyrus-psc-files", type=str, help="the papyrus source files", nargs="+", required=False)
	parser.add_argument("--plugin-files", type=str, help="the files to archive", nargs="+", required=True)
	parser.add_argument("--config-files", type=str, help="the config files to archive", nargs="+", required=False)
	parser.add_argument("--pdbs", type=str, help="the pdbs to archive", nargs="+", required=True)
	parser.add_argument("--project", type=str, help="the project's name", required=True)
	return parser.parse_args()

def main():
	print("Archiving artifacts...")

	args = parse_arguments()

	out = os.path.join(args.bin_dir, "artifacts")
	os.makedirs(out, exist_ok=True)
	os.chdir(out)

	make_rel_archive(args.project, args)
	make_dbg_archive(args.project, args)

if __name__ == "__main__":
	main()
