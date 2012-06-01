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
import java.net.URL;
import java.util.ArrayList;
import java.util.Properties;
import java.util.StringTokenizer;

import edu.cmu.meteor.aligner.Aligner;
import edu.cmu.meteor.aligner.Alignment;
import edu.cmu.meteor.aligner.Match;
import edu.cmu.meteor.scorer.MeteorConfiguration;
import edu.cmu.meteor.scorer.MeteorScorer;
import edu.cmu.meteor.scorer.MeteorStats;
import edu.cmu.meteor.util.Constants;

public class MMBRMatcherMEMT {
	public static void main(String[] args) throws Exception {

		// Usage
		if (args.length < 1) {
			System.out.println("METEOR Aligner and MMBR for MEMT version "
					+ Constants.VERSION);
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
			if (args[0].charAt(0) == '-') {
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
		if (modNames.equals("default"))
			modNames = "exact stem synonym paraphrase";
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

		// Construct Scorer only (no Aligner modules)
		MeteorConfiguration noAligner = new MeteorConfiguration();
		noAligner.setTask("hter");
		noAligner.setModules(new ArrayList<Integer>());
		MeteorScorer scorer = new MeteorScorer(noAligner);

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
			if (lines.get(0) == null)
				break;
			handleLines(aligner, moduleWeights, scorer, lines);
		}
	}

	static void handleLines(Aligner aligner, ArrayList<Double> moduleWeights,
			MeteorScorer scorer, ArrayList<String> lines) {
		System.out.println(lines.size());
		for (String l : lines) {
			System.out.println(l);
		}
		for (int i = 0; i < lines.size(); ++i) {
			for (int j = i + 1; j < lines.size(); ++j) {

				// Align
				Alignment alignment = aligner.align(lines.get(i), lines.get(j));

				// Direction 1 precision, recall, chunks, score
				MeteorStats stats = scorer.getMeteorStats(alignment);
				System.out.println(stats.precision + "\t" + stats.recall + "\t"
						+ stats.chunks + "\t" + stats.score);

				// Direction 2 precision, recall, chunks, score
				swap(stats);
				scorer.computeMetrics(stats);
				System.out.println(stats.precision + "\t" + stats.recall + "\t"
						+ stats.chunks + "\t" + stats.score);

				// Matches
				printAlignment(moduleWeights, alignment);
				System.out.println();

			}
		}
	}

	static void swap(MeteorStats stats) {
		stats.testWeightedMatches = 0;
		stats.referenceWeightedMatches = 0;
		int testLength = stats.testLength;
		stats.testLength = stats.referenceLength;
		stats.referenceLength = testLength;
		int testTotalMatches = stats.testTotalMatches;
		stats.testTotalMatches = stats.referenceTotalMatches;
		stats.referenceTotalMatches = testTotalMatches;
		ArrayList<Integer> testTotalStageMatches = stats.testTotalStageMatches;
		stats.testTotalStageMatches = stats.referenceTotalStageMatches;
		stats.referenceTotalStageMatches = testTotalStageMatches;
		ArrayList<Double> testWeightedStageMatches = stats.testWeightedStageMatches;
		stats.testWeightedStageMatches = stats.referenceWeightedStageMatches;
		stats.referenceWeightedStageMatches = testWeightedStageMatches;
	}

	static void printAlignment(ArrayList<Double> moduleWeights,
			Alignment alignment) {
		for (int i = 0; i < alignment.matches.size(); ++i) {
			Match m = alignment.matches.get(i);
			if (m.matchStringStart != -1) {
				// First string word
				System.out.print(m.matchStringStart + "\t");
				// Second string word
				System.out.print(m.start + "\t");
				// Module stage
				System.out.print(m.stage + "\t");
				// Score
				System.out.println(m.prob * moduleWeights.get(m.stage));
			}
		}
	}
}
