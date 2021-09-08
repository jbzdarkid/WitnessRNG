It turns out that "random" in games doesn't really mean "any possible puzzle" -- due to the way computers work, most randomization is actually just a choice between `2^32` different random seeds.

And, (at least for some of the puzzles in TW), certain seeds correspond to unsolvable puzzles, so the game re-rolls them -- that is to say, it generates another puzzle (so that the player can actually *beat* the challenge).

Anyways, a while back, tzann & I reproduced the RNG from the game, and this is the next step along that journey -- reproducing entire puzzle generation. Specifically, I'm starting with the polyomino puzzle, which we've *already* noticed duplications in, just using our human-level brains. So there's __definitely__ a small search space here, which means we could, potentially, learn all the puzzles -- or at least find the common patterns. 

The good news is, the RNG generation works, and I can make the same puzzle as the game for the same seed. The bad news is, `2^32` puzzles is *rather a lot*, and the only solver we have that's __100% accurate__ is [written in javascript](https://github.com/jbzdarkid/witness-puzzles), which is not particularly fast. By my back-of-the-hand estimation, it would take about 15,000 CPU hours to find all the solutions -- and I'm not even sure what we'd *do* with that information, exactly, since it's around 2 TB of raw data for *just the puzzles*, not counting the solutions. Though it does compress well, but still.

So, I'm pretty sure the next step on this journey is porting that engine into C++. Ideally here, in this repo. And then generating and solving *all the puzzles*. And then figuring out what's the most common type, and learning all the patterns. And then probably making a video or something.
