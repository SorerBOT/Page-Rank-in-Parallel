# Page-Rank-in-Parallel
Well... a parallel implementation of Google's PageRank algorithm.

## What the heck is Page Rank?
When surfing the internet, we need a way to determine which results are the most relevant to the user. In order to so we need to be able to RANK webPAGEs, hence PageRank. The jist of it, is that the more a website is "cited", or linked to, the more it is relevant. From here, the rest of the logic is destined to determine which links are more worthwhile, and credit the website to which they link with a higher rank.

The algorithm which calculates how "cited" a website is, emulates a random surfer travelling between websites via weblinks or entering search queries in different probabilities.

## Notations
* $d := 0.85$ (the damping factor) the probability that the user continues surfing (moving from one website to another via a link).
* $1-d$ the probability that the user makes a new search instead of surfing.
* $N$ - the total number of websites.
* $u_i$ - the websites that link to a site $v$.
* $u_j$ - the websites that don't link to any other websites.
* $\text{Outlink}(v)$ - the number of websites to which $v$ links.

## The formula
Because we want to get a probability function, then we have to slightly tweak the first iteration. For this reason, we actually end up with two formulas:

#### First Iteration
$$\text{PageRank}(v)=\frac{1}{N}$$
#### Any Other Iteration
$$\text{PageRank}(v) := \frac{1-d}{N} + d \cdot \left(\sum_{u_i} \frac{\text{PageRank}(u_i)}{\text{Outlink}(u_i)}+ \sum_{u_j} \frac{\text{PageRank}(u_j)}{N}\right)
$$

* Each element in the left sum, represents the importance of the link from $u_i$ to $v$. The more important $u_i$ is, the more important the link is. Additionally, if $u_i$ points to a ton of websites, the importance of $u_i$'s links are split amongst all the websites to which it points.
* The second sum is meant to consider the cases in which we have arrived at a website that has no outlinks, and hence we'd have to make a new search in order to continue.

## Proof that it is a proper probability function
We will prove that for every iteration the sum of PageRanks of all nodes is $1$. To do so, we will use induction:

#### Base case:
When $i=1$, we defined the rank of every node to be $\frac{1}{N}$, and so the sum of the ranks is $N\cdot \frac{1}{N}$ which is of course $1$, as needed.

#### Induction Step:
We will assume that the hypothesis holds for every iteration until the $k^{\text{th}}$ iteration, and prove that it holds for the next.

If we distribute the sum, we get $$\sum_{i=1}^N \text{PageRank}(v) = \sum_{i=1}^N \left( \frac{1-d}{N} + d \left( \cdot \sum_{u_i} \frac{\text{PageRank}(u_i)}{\text{Outlink}(u_i)}+ \sum_{u_j} \frac{\text{PageRank}(u_j)}{N}\right)\right)$$


$$\underbrace{\sum_{i=1}^N (\frac{1-d}{N})}_{\text{1-d}} + d \cdot \sum_{i=1}^N( \sum_{u_i} \frac{\text{PageRank}(u_i)}{\text{Outlink}(u_i)}) + d \cdot \sum_{i=1}^N(\sum_{u_j} \frac{\text{PageRank}(u_j)}{N})$$

And if we look closer at the sums at the right side, we notice that for every page $u_i$, that has links to other pages, the amount of PageRanks that we assign to each of the pages to which it links is:

$$
\frac{\text{PageRank}(u_i)}{\text{Outlink}(u_i)}
$$

But $u_i$ has $\text{Outlink}(u_i)$ websites that it links to, and so the sum of the PageRanks that we give to the websites to which it links is:



$$ \frac{\text{PageRank}(u_i)}{\text{Outlink}(u_i)} \cdot \text{Outlink}(u_i) = \text{PageRank}(u_i) $$



And if we sum the amount of PageRanks that we give to the ALL pages, for each website that doesn't link to any other page, we get:


$$
\frac{\text{PageRank}(u_j)}{N} \cdot N = \text{PageRank}(u_j)
$$

And so the sum, of the two rightmost sums is exactly $\sum_{u\in G}{\text{PageRank}(u)}$ which we assumed to be $1$ in the previous iteration, and so we get:


$$\underbrace{\sum_{i=1}^N (\frac{1-d}{N})}_{\text{1-d}} + \underbrace{d \cdot \sum_{i=1}^N( \sum_{u_i} \frac{\text{PageRank}(u_i)}{\text{Outlink}(u_i)}) + d \cdot \sum_{i=1}^N(\sum_{u_j} \frac{\text{PageRank}(u_j)}{N})}_{d\cdot 1}=1$$
