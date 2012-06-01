/*
 * Carnegie Mellon University
 * Copyright (c) 2004, 2009
 * All Rights Reserved.
 *
 * Any use of this software must follow the terms
 * outlined in the included LICENSE file.
 */

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.PrintStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Properties;
import java.util.StringTokenizer;

import edu.cmu.meteor.aligner.Aligner;
import edu.cmu.meteor.aligner.Alignment;
import edu.cmu.meteor.aligner.Match;
import edu.cmu.meteor.util.Constants;

public class MatcherMEMT {
	public static void main(String[] args) throws Exception {

		// Usage
		if (args.length < 1) {
			System.out.println("METEOR Aligner for MEMT version " + Constants.VERSION);
			System.out.println("Usage: java -cp meteor.jar MatcherMEMT "
					+ "<file1> <file2> ... <filen> [options]");
			System.out.println();
			System.out.println("Options:");
			System.out.println("-l language\t\t\tOne of: en cz de es fr");
			System.out
					.println("-m \"module1 module2 ...\"\tSpecify modules (overrides default)");
			System.out
					.println("\t\t\t\t  One of: exact stem synonym paraphrase");
			System.out.println("-x maxComputations\t\tKeep speed reasonable");
			System.out.println("-d synonymDirectory\t\t(if not default)");
			System.out.println("-a paraphraseFile\t\t(if not default)");
			System.out.println();
			System.out
					.println("Default settings are stored in the matcher.properties file bundled in the JAR");
			return;
		}

		// Defaults
		String propFile = "matcher.properties";
		Properties props = new Properties();
		try {
			props.load(ClassLoader.getSystemResource(propFile).openStream());
		} catch (Exception ex) {
			System.err.println("Error: Could not load properties file:");
			ex.printStackTrace();
			System.exit(1);
		}

    ArrayList<BufferedReader> files = new ArrayList<BufferedReader>();
    int curArg;
    for (curArg = 0; curArg < args.length; ++curArg) {
      if (args[curArg].charAt(0) == '-') {
        break;
      }
			files.add(new BufferedReader(new FileReader(args[curArg])));
    }

		// Options
		while (curArg < args.length) {
			if (args[curArg].equals("-l")) {
				props.setProperty("language", args[curArg + 1]);
				curArg += 2;
			} else if (args[curArg].equals("-x")) {
				props.setProperty("maxcomp", args[curArg + 1]);
				curArg += 2;
			} else if (args[curArg].equals("-d")) {
				props.setProperty("synDir", args[curArg + 1]);
				curArg += 2;
			} else if (args[curArg].equals("-a")) {
				props.setProperty("paraDir", args[curArg + 1]);
				curArg += 2;
			} else if (args[curArg].equals("-m")) {
				props.setProperty("modules", args[curArg + 1]);
				curArg += 2;
			} else {
        System.err.println("Bad argument " + args[curArg]);
        System.exit(1);
      }
		}

		// Language
		String language = props.getProperty("language");
		if (language.equals("default"))
			language = "english";
		language = Constants.normLanguageName(language);

		// Synonym Location
		String synDir = props.getProperty("synDir");
		URL synURL;
		if (synDir.equals("default"))
			synURL = Constants.DEFAULT_SYN_DIR_URL;
		else
			synURL = (new File(synDir)).toURI().toURL();

		// Paraphrase Location
		String paraDir = props.getProperty("paraDir");
		URL paraURL;
		if (paraDir.equals("default"))
			paraURL = Constants.DEFAULT_PARA_DIR_URL;
		else
			paraURL = (new File(paraDir)).toURI().toURL();

		// Max Computations
		String mx = props.getProperty("maxcomp");
		int maxComp = 0;
		if (mx.equals("default"))
			maxComp = Constants.DEFAULT_MAXCOMP;
		else
			maxComp = Integer.parseInt(mx);

		// Modules
		String modNames = props.getProperty("modules");
		if (modNames.equals("default")) {
      modNames = "exact stem synonym paraphrase";
    }
		ArrayList<Integer> modules = new ArrayList<Integer>();
		ArrayList<Double> moduleWeights = new ArrayList<Double>();
		StringTokenizer mods = new StringTokenizer(modNames);
		while (mods.hasMoreTokens()) {
			int module = Constants.getModuleID(mods.nextToken());
			modules.add(module);
			moduleWeights.add(1.0);
		}

		// Construct aligner
		Aligner aligner = new Aligner(language, modules, moduleWeights,
				maxComp, synURL, paraURL);

    PrintStream bufferedOut = new PrintStream(System.out, false);
    while (true) {
      ArrayList<String> lines = new ArrayList<String>(files.size());
      for (BufferedReader f : files) {
        lines.add(f.readLine());
      }
      for (String l : lines) {
        if ((lines.get(0) == null) != (l == null)) {
          System.err.println("Files are not equal length");
          System.exit(1);
        }
      }
      if (lines.get(0) == null) break;
      handleLines(modules, aligner, moduleWeights, lines, bufferedOut);
    }
  }

  static void handleLines(ArrayList<Integer> modules, Aligner aligner, ArrayList<Double> moduleWeights, ArrayList<String> lines, PrintStream to) {
    to.println(lines.size());
    for (String l : lines) {
      to.println(l);
    }
    for (int i = 0; i < lines.size(); ++i) {
      for (int j = i + 1; j < lines.size(); ++j) {
        printAlignment(modules, moduleWeights, aligner.align(lines.get(i), lines.get(j)), to);
      }
    }
  }

  static void printAlignment(ArrayList<Integer> modules, ArrayList<Double> moduleWeights, Alignment alignment, PrintStream to) {
		for (int i = 0; i < alignment.matches.size(); ++i) {
			Match m = alignment.matches.get(i);
			if (m.matchStringStart != -1) {
				// First string word
				to.print(m.matchStringStart + "\t");
				// Second string word
				to.print(m.start + "\t");
				// Module stage
				to.print(modules.get(m.stage) + "\t");
				// Score
				to.println(m.prob * moduleWeights.get(m.stage));
			}
		}
    to.println();
  }
}
